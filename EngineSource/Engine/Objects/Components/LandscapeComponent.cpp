#include "LandscapeComponent.h"
#include <Engine/Objects/SceneObject.h>
#include <Engine/Scene.h>
#include <Engine/Physics/Physics.h>
#include <Engine/Debug/TimeLogger.h>
#include <Engine/Graphics/VideoSubsystem.h>
#include <Core/ThreadPool.h>
#include <Engine/Input.h>

using namespace engine::graphics;
using namespace engine;

constexpr uint32 SIZE = 128;

void engine::LandscapeComponent::OnAttached()
{
	Data = std::make_shared<LandscapeData>(16, 16);
	Generator = std::make_shared<LandscapeMeshGenerator>();
	Generator->Data = Data;

	size_t NumChunks = Data->Width * Data->Height;
	{
		Generate();
	}
	//for (auto& i : Vertices)
	//{
	//	i.Normal = Vector3(0);
	//}

	//for (size_t i = 0; i < Indices.size(); i += 3)
	//{
	//	size_t A = Indices[i], B = Indices[i + 1], C = Indices[i + 2];
	//	Vector3 n = Vector3::Cross(Vertices[B].Position - Vertices[A].Position, Vertices[C].Position - Vertices[A].Position);
	//	Vertices[A].Normal += n;
	//	Vertices[B].Normal += n;
	//	Vertices[C].Normal += n;
	//}
	//for (auto& v : Vertices)
	//{
	//	v.Normal = v.Normal.Normalize();
	//}

	std::vector<float> Samples;
	Samples.resize(SIZE * SIZE);
	Collider = new physics::HeightMapBody(Samples, SIZE, WorldTransform,
		physics::MotionType::Static, physics::Layer::Static, this);

	GetRootObject()->GetScene()->Physics.AddBody(Collider, true, true);
	GetRootObject()->GetScene()->Graphics.AddDrawnComponent(this);
}

void engine::LandscapeComponent::OnDetached()
{
	GetRootObject()->GetScene()->Physics.RemoveBody(Collider);
	GetRootObject()->GetScene()->Graphics.RemoveDrawnComponent(this);
}

void engine::LandscapeComponent::Draw(graphics::Renderer* Render, graphics::Camera* From, graphics::GraphicsScene* In)
{
	auto Pass = Render->StartRender();
	auto Root = GetRootObject();
	auto Scene = Root ? Root->GetScene() : nullptr;
	LandscapeMaterial->Apply(Pass);
	ShaderObject* Used = LandscapeMaterial->Shader;
	if (!Used)
	{
		return;
	}

	if (!Used->Unlit)
	{
		In->Lights.ApplyToShader(Used, DrawBoundingBox);
		In->Shadows.BindUniforms(Pass, Used);
	}
	From->UsedEnvironment->ApplyTo(Used);

	Used->SetMatrix(Used->ModelUniform, WorldTransform.Matrix);
	Used->SetMatrix(Used->GetUniformLocation("u_view"), From->View);
	Used->SetMatrix(Used->GetUniformLocation("u_projection"), From->Projection);
	Used->SetVec3(Used->GetUniformLocation("u_cameraPos"), From->GetPosition());
	Pass->SetStencilValue(DrawStencil, 1);

	if (RootSegment)
	{
		RootSegment->Draw(Pass, Used, From);
	}
}

void engine::LandscapeComponent::SimpleDraw(graphics::Renderer* Render, graphics::ShaderObject* With)
{
	auto Pass = Render->StartRender();
	this->LandscapeMaterial->ApplySimple(Pass, With);
	With->SetMatrix(With->ModelUniform, WorldTransform.Matrix);

	if (RootSegment)
	{
		RootSegment->Draw(Pass, nullptr);
	}
}

void engine::LandscapeComponent::Update()
{
	if (input::IsKeyHeld(input::Key::SPACE))
	{
		return;
	}

	if (Generator->IsDone)
	{
		Generator->IsDone = false;
		if (this->RootSegment)
		{
			delete this->RootSegment;
		}
		this->RootSegment = Generator->RootSegment;
		this->RootSegment->CreateVertexBuffer(this->WorldTransform);
		CanGenerate = true;
	}

	if (CanGenerate)
	{
		Generator->CameraPosition = WorldTransform.Inverse().ApplyTo(GetRootObject()->GetScene()->Graphics.UsedCamera->Position);
		if (CheckLod(RootSegment))
		{
			Generate();
		}
		else if (IsDirty)
		{
			Generate();
		}
	}
}

bool engine::LandscapeComponent::UpdateTransform(bool Dirty)
{
	bool Result = ObjectComponent::UpdateTransform(Dirty);
	if (Result)
	{
		this->DrawBoundingBox = BoundingBox(Vector3(SIZE) * Vector3(1, 0, 1), SIZE).Translate(WorldTransform);
		this->IsDirty = true;
	}
	return Result;
}

static void GenerateThread(std::shared_ptr<LandscapeMeshGenerator> Generator,
	std::shared_ptr<LandscapeData> Data)
{
	Generator->RootSegment = Generator->BuildSegment(0, 0, Generator->Data->Width);
	Generator->MergeSegments(Generator->RootSegment, nullptr, nullptr);
	Generator->BuildSegments(Generator->RootSegment, 0, 0, Generator->Data->Width);
	Generator->IsDone = true;
}

void engine::LandscapeComponent::Generate()
{
	ThreadPool::Main()->AddJob(std::bind(GenerateThread, this->Generator, this->Data));
	CanGenerate = false;
	IsDirty = false;
}

bool engine::LandscapeComponent::CheckLod(LandscapeSegment* ForSegment)
{
	auto CalcScale = Generator->CalculateLodScale(ForSegment->ChunkX, ForSegment->ChunkY, ForSegment->Scale);
	if (ForSegment->SegmentMesh)
	{
		return ForSegment->Scale >= CalcScale;
	}

	for (auto& i : ForSegment->SubSegments)
	{
		bool Result = CheckLod(i);

		if (Result)
		{
			return true;
		}
	}

	return ForSegment->Scale < CalcScale;
}

LandscapeSegment* engine::LandscapeMeshGenerator::BuildSegment(size_t X, size_t Y, size_t Scale)
{
	if (Scale < CalculateLodScale(X, Y, Scale))
	{
		return new LandscapeSegment(Data->CombineChunks(X * LANDSCAPE_CHUNK_SIZE, Y * LANDSCAPE_CHUNK_SIZE, Scale),
			X, Y, Scale);
	}

	auto SuperSegment = new LandscapeSegment(X, Y, Scale);

	size_t NewScale = Scale / 2;

	SuperSegment->SubSegments[0] = BuildSegment(X, Y + NewScale, NewScale);
	SuperSegment->SubSegments[1] = BuildSegment(X + NewScale, Y + NewScale, NewScale);
	SuperSegment->SubSegments[2] = BuildSegment(X, Y, NewScale);
	SuperSegment->SubSegments[3] = BuildSegment(X + NewScale, Y, NewScale);
	return SuperSegment;
}

void engine::LandscapeMeshGenerator::MergeSegments(LandscapeSegment* From, LandscapeSegment* Left, LandscapeSegment* Bottom)
{
	if (From->PlaceholderChunk)
	{
		if (Bottom && From->ChunkY > 0)
		{
			auto BottomSegment = GetSegment(Bottom, From->ChunkX, From->ChunkY - 1);

			if (BottomSegment && BottomSegment->Scale > From->Scale)
			{
				From->BottomScaleDifference = BottomSegment->Scale / From->Scale;
				for (size_t x = 0; x < LANDSCAPE_CHUNK_SIZE - BottomSegment->Scale; x += From->BottomScaleDifference)
				{
					LandscapePoint p = From->PlaceholderChunk->Points[x];
					p.Interpolate(From->PlaceholderChunk->Points[x + From->BottomScaleDifference], 0.5f);
					From->PlaceholderChunk->Points[x + 1] = p;
				}
			}
		}
		if (Left && From->ChunkX > 0)
		{
			auto LeftSegment = GetSegment(Left, From->ChunkX - 1, From->ChunkY);

			if (LeftSegment && LeftSegment->Scale > From->Scale)
			{
				From->LeftScaleDifference = LeftSegment->Scale / From->Scale;
				for (size_t y = 0; y < LANDSCAPE_CHUNK_SIZE - LeftSegment->Scale; y += From->LeftScaleDifference)
				{
					LandscapePoint p = From->PlaceholderChunk->Points[y * LANDSCAPE_CHUNK_SIZE];
					p.Interpolate(From->PlaceholderChunk->Points[(From->LeftScaleDifference + y) * LANDSCAPE_CHUNK_SIZE], 0.5f);
					From->PlaceholderChunk->Points[(1 + y) * LANDSCAPE_CHUNK_SIZE] = p;
				}

			}
		}

		return;
	}

	MergeSegments(From->SubSegments[0], Left, From->SubSegments[2]);
	MergeSegments(From->SubSegments[1], From->SubSegments[0], From->SubSegments[3]);
	MergeSegments(From->SubSegments[2], Left, Bottom);
	MergeSegments(From->SubSegments[3], From->SubSegments[2], Bottom);
}

void engine::LandscapeMeshGenerator::BuildSegments(LandscapeSegment* From, size_t X, size_t Y, size_t Scale)
{
	if (From->PlaceholderChunk)
	{
		LandscapeSegment* Chunks[4] = {
			nullptr, nullptr, nullptr, nullptr
		};

		Chunks[0] = From;

		if (X + Scale < Data->Width)
		{
			Chunks[1] = GetSegment(RootSegment, X + Scale, Y);
		}
		if (Y + Scale < Data->Height)
		{
			Chunks[2] = GetSegment(RootSegment, X, Y + Scale);
		}
		if (Y + Scale < Data->Height && X + Scale < Data->Width)
		{
			Chunks[3] = GetSegment(RootSegment, X + Scale, Y + Scale);
		}
		From->BuildBuffer(Chunks, this);
	}
	else
	{
		for (auto& i : From->SubSegments)
		{
			BuildSegments(i, i->ChunkX, i->ChunkY, Scale / 2);
		}
	}
}

LandscapeSegment* engine::LandscapeMeshGenerator::GetSegment(LandscapeSegment* From, size_t ChunkX, size_t ChunkY)
{
	if (ChunkX >= From->ChunkX && ChunkX < From->ChunkX + From->Scale
		&& ChunkY >= From->ChunkY && ChunkY < From->ChunkY + From->Scale)
	{
		if (From->PlaceholderChunk || From->SegmentMesh)
		{
			return From;
		}

		for (auto& i : From->SubSegments)
		{
			auto Result = GetSegment(i, ChunkX, ChunkY);

			if (Result)
			{
				return Result;
			}
		}
	}

	return nullptr;
}

size_t engine::LandscapeMeshGenerator::CalculateLodScale(size_t X, size_t Y, size_t Scale) const
{
	Vector3 SegmentPosition = Vector3(X, 0.0f, Y);
	float SegmentScale = float(Scale) / 2.0f;

	SegmentPosition += Vector3(SegmentScale, 0, SegmentScale);

	float Distance = Vector3::Distance(this->CameraPosition / LANDSCAPE_CHUNK_SIZE, SegmentPosition) - SegmentScale;

	return std::max(Distance / 1.5f, 2.0f);
}

engine::LandscapeSegment::LandscapeSegment(size_t X, size_t Y, size_t Scale)
{
	this->ChunkX = X;
	this->ChunkY = Y;
	this->Scale = Scale;
}

engine::LandscapeSegment::LandscapeSegment(LandscapeChunk* PlaceholderChunk, size_t X, size_t Y, size_t Scale)
{
	this->PlaceholderChunk = PlaceholderChunk;

	this->ChunkX = X;
	this->ChunkY = Y;
	this->Scale = Scale;
}

void engine::LandscapeSegment::BuildBuffer(LandscapeSegment* Chunks[4], LandscapeMeshGenerator* Generator)
{
	size_t MaxX = Chunks[1] ? LANDSCAPE_CHUNK_SIZE + 1 : LANDSCAPE_CHUNK_SIZE;
	size_t MaxY = Chunks[2] ? LANDSCAPE_CHUNK_SIZE + 1 : LANDSCAPE_CHUNK_SIZE;
	Vertices.reserve(MaxX * MaxY);

	LandscapeSegment* RightSegment = Chunks[1];
	LandscapeSegment* TopSegment = Chunks[2];

	size_t CornerItemOffset = 0;
	size_t BottomSizeScale = (BottomScaleDifference - 1);
	size_t LeftSizeScale = (LeftScaleDifference - 1);

	if (Chunks[3])
	{
		size_t DiffX = (ChunkX + Scale - Chunks[3]->ChunkX) * LANDSCAPE_CHUNK_SIZE / Chunks[3]->Scale;
		size_t DiffY = (ChunkY + Scale - Chunks[3]->ChunkY) * LANDSCAPE_CHUNK_SIZE / Chunks[3]->Scale;

		CornerItemOffset = DiffX + DiffY * LANDSCAPE_CHUNK_SIZE;
	}

	auto SamplePointX = [Chunks, CornerItemOffset](LandscapeSegment* Segment, size_t X, size_t Scale) {
		if (Scale != 1)
		{
			float Offset = float(X % Scale) / float(Scale);

			auto& p = Segment->PlaceholderChunk->Points[X / Scale];

			if (Offset == 0.0f)
			{
				return p;
			}

			if (X / Scale + 1 >= LANDSCAPE_CHUNK_SIZE)
			{
				if (!Chunks[3])
				{
					return p;
				}

				LandscapePoint Result = p;
				Result.Interpolate(Chunks[3]->PlaceholderChunk->Points[CornerItemOffset], Offset);
				return Result;
			}

			LandscapePoint Result = p;
			Result.Interpolate(Segment->PlaceholderChunk->Points[X / Scale + 1], Offset);

			return Result;
		}

		return Segment->PlaceholderChunk->Points[X];
	};

	auto SamplePointY = [Chunks, CornerItemOffset](LandscapeSegment* Segment, size_t Y, size_t Scale) {
		if (Scale != 1)
		{
			float Offset = float(Y % Scale) / float(Scale);

			auto& p = Segment->PlaceholderChunk->Points[Y / Scale * LANDSCAPE_CHUNK_SIZE];

			if (Offset == 0.0f)
			{
				return p;
			}

			if (Y / Scale + 1 >= LANDSCAPE_CHUNK_SIZE)
			{
				if (!Chunks[3])
				{
					return p;
				}

				LandscapePoint Result = p;
				Result.Interpolate(Chunks[3]->PlaceholderChunk->Points[CornerItemOffset], Offset);
				return Result;
			}

			LandscapePoint Result = p;
			Result.Interpolate(Segment->PlaceholderChunk->Points[(Y / Scale + 1) * LANDSCAPE_CHUNK_SIZE], Offset);

			return Result;
		}

		return Segment->PlaceholderChunk->Points[Y * LANDSCAPE_CHUNK_SIZE];
	};

	for (uint32 y = 0; y < MaxY; y++)
	{
		for (uint32 x = 0; x < MaxX; x++)
		{
			uint32 i = x + y * MaxX;

			LandscapePoint p;

			if (x < LANDSCAPE_CHUNK_SIZE && y < LANDSCAPE_CHUNK_SIZE)
			{
				if (RightSegment && x + BottomSizeScale >= LANDSCAPE_CHUNK_SIZE && y < 1)
				{
					p = Chunks[0]->PlaceholderChunk->Points[x > 0 ? x - 1 : 0];
					size_t ChunkDiff = (ChunkY - RightSegment->ChunkY) * LANDSCAPE_CHUNK_SIZE;
					auto p2 = SamplePointY(RightSegment, ChunkDiff + y * this->Scale, RightSegment->Scale);

					p.Interpolate(p2, 0.5f);
				}
				else if (TopSegment && y + LeftSizeScale >= LANDSCAPE_CHUNK_SIZE && x < 1)
				{
					p = Chunks[0]->PlaceholderChunk->Points[(y - 1) * LANDSCAPE_CHUNK_SIZE];
					size_t ChunkDiff = (ChunkX - TopSegment->ChunkX) * LANDSCAPE_CHUNK_SIZE;
					auto p2 = SamplePointX(TopSegment, ChunkDiff + x * this->Scale, TopSegment->Scale);

					p.Interpolate(p2, 0.5f);
				}
				else
				{
					p = Chunks[0]->PlaceholderChunk->Points[x + y * LANDSCAPE_CHUNK_SIZE];
				}
			}
			else if (x >= LANDSCAPE_CHUNK_SIZE && y < LANDSCAPE_CHUNK_SIZE)
			{
				size_t ChunkPos = (y * Scale + ChunkY * LANDSCAPE_CHUNK_SIZE) / LANDSCAPE_CHUNK_SIZE;

				if (ChunkPos >= RightSegment->ChunkY + RightSegment->Scale)
				{
					RightSegment = Generator->GetSegment(Generator->RootSegment, RightSegment->ChunkX, ChunkPos);
				}

				size_t ChunkDiff = (ChunkY - RightSegment->ChunkY) * LANDSCAPE_CHUNK_SIZE;
				p = SamplePointY(RightSegment, ChunkDiff + y * this->Scale, RightSegment->Scale);
			}
			else if (x < LANDSCAPE_CHUNK_SIZE && y >= LANDSCAPE_CHUNK_SIZE)
			{
				size_t ChunkPos = (x * Scale + ChunkX * LANDSCAPE_CHUNK_SIZE) / LANDSCAPE_CHUNK_SIZE;

				if (ChunkPos >= TopSegment->ChunkX + TopSegment->Scale)
				{
					TopSegment = Generator->GetSegment(Generator->RootSegment, ChunkPos, TopSegment->ChunkY);
				}

				size_t ChunkDiff = (ChunkX - TopSegment->ChunkX) * LANDSCAPE_CHUNK_SIZE;
				p = SamplePointX(TopSegment, ChunkDiff + x * this->Scale, TopSegment->Scale);
			}
			else
			{
				p = Chunks[3]->PlaceholderChunk->Points[CornerItemOffset];
			}

			Vertices.emplace_back(Vector3(
				x * Scale + ChunkX * LANDSCAPE_CHUNK_SIZE,
				p.Height,
				y * Scale + ChunkY * LANDSCAPE_CHUNK_SIZE),
				Vector2(x * Scale, y * Scale), p.Normal);

			if (x < MaxX - 1 && y < MaxY - 1)
			{
				Indices.push_back(i + 0);
				Indices.push_back(i + MaxX);
				Indices.push_back(i + MaxX + 1);
				Indices.push_back(i + 0);
				Indices.push_back(i + MaxX + 1);
				Indices.push_back(i + 1);
			}
		}
	}
}


void engine::LandscapeSegment::Draw(graphics::DrawCommand* Pass, ShaderObject* WithShader)
{
	if (SegmentMesh)
	{
		Pass->DrawVertexBuffer(SegmentMesh);
	}
	else
	{
		for (auto& i : SubSegments)
		{
			i->Draw(Pass, WithShader);
		}
	}
}

void engine::LandscapeSegment::Draw(graphics::DrawCommand* Pass, graphics::ShaderObject* WithShader, graphics::Camera* Cam)
{
	if (Cam->Collider.OverlapsBounds(this->Bounds))
	{
		if (SegmentMesh)
		{
			Pass->DrawVertexBuffer(SegmentMesh);
		}
		else
		{
			for (auto& i : SubSegments)
			{
				i->Draw(Pass, WithShader, Cam);
			}
		}
	}
}

void engine::LandscapeSegment::CreateVertexBuffer(const Transform& WithTransform)
{
	if (this->PlaceholderChunk)
	{
		SegmentMesh = VideoSubsystem::Current->Renderer->CreateVertexBuffer(Vertices, Indices);
		Vertices.clear();
		Indices.clear();
		delete this->PlaceholderChunk;
		PlaceholderChunk = nullptr;
	}
	else
	{
		for (auto& i : this->SubSegments)
		{
			i->CreateVertexBuffer(WithTransform);
		}
	}
	Vector3 SegmentPosition = Vector3(ChunkX, 0.0f, ChunkY) * LANDSCAPE_CHUNK_SIZE;
	float SegmentScale = float(Scale * LANDSCAPE_CHUNK_SIZE) / 2.0f;

	this->Bounds = BoundingBox(SegmentPosition + Vector3(SegmentScale, 0, SegmentScale), Vector3(SegmentScale, 100.0f, SegmentScale)).Translate(WithTransform);
}
