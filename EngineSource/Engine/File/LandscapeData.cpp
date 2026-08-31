#include "LandscapeData.h"
#include <miniz.h>
#include <Engine/File/Resource.h>
#include <Core/Log.h>

using namespace engine;

engine::LandscapeData::LandscapeData(size_t Width, size_t Height)
{
	this->Width = Width;
	this->Height = Height;
	this->Chunks.resize(Width * Height);

	for (size_t x = 0; x < Width; x++)
	{
		for (size_t y = 0; y < Height; y++)
		{
			Chunks[x + y * Height].Init(x, y);
		}
	}
	InitializeNormal();
}

engine::LandscapeData::LandscapeData(AssetRef Asset)
{
	if (!Asset.Exists())
	{
		return;
	}

	IBinaryStream* File = resource::GetBinaryFile(Asset.FilePath);

	if (File->IsEmpty())
	{
		delete File;
		return;
	}

	if (File->ReadString() != "hmp/0")
	{
		delete File;
		return;
	}

	Width = File->Get<uint64>();
	Height = File->Get<uint64>();

	Chunks.resize(File->Get<uint64>());
	mz_ulong CompressedSize = File->Get<uint64>();

	uByte* Buffer = new uByte[CompressedSize];
	File->Read(Buffer, CompressedSize);

	mz_ulong BufferLength = this->Chunks.size() * sizeof(LandscapeChunk);

	mz_uncompress((uByte*)Chunks.data(), &BufferLength, Buffer, CompressedSize);
	delete[] Buffer;
	delete File;
}

engine::LandscapeData::LandscapeData()
{
}

engine::LandscapeData::~LandscapeData()
{
	if (Collider)
	{
		delete Collider;
	}
}

void engine::LandscapeData::InitializeNormal()
{
	InitializeNormalForRange(-1, -1, Width * LANDSCAPE_CHUNK_SIZE + 1, Width * LANDSCAPE_CHUNK_SIZE + 1);
}

void engine::LandscapeData::LoadCollider(ObjectComponent* Component)
{
	if (Collider)
	{
		delete Collider;
	}
	Collider = new physics::HeightMapBody(this, Transform(), physics::MotionType::Static, physics::Layer::Static, Component);
}

void engine::LandscapeData::InitializeNormalForRange(int64 X, int64 Y, int64 W, int64 H)
{
	W += X;
	H += Y;

	for (int64 x = X + 1; x < std::min(int64(Width * LANDSCAPE_CHUNK_SIZE), W - 1); x++)
	{
		for (int64 y = Y + 1; y < std::min(int64(Height * LANDSCAPE_CHUNK_SIZE), H - 1); y++)
		{
			if (x >= Width * LANDSCAPE_CHUNK_SIZE - 1 || y >= Height * LANDSCAPE_CHUNK_SIZE - 1)
			{
				continue;
			}

			if (x < 0 || y < 0)
			{
				continue;
			}

			GetPointAt(x, y).Normal = 0;
		}
	}

	for (int64 x = X; x < std::min(int64(Width * LANDSCAPE_CHUNK_SIZE), W); x++)
	{
		for (int64 y = Y; y < std::min(int64(Height * LANDSCAPE_CHUNK_SIZE), H); y++)
		{
			if (x >= Width * LANDSCAPE_CHUNK_SIZE - 1 || y >= Height * LANDSCAPE_CHUNK_SIZE - 1)
			{
				continue;
			}

			if (x < 0 || y < 0)
			{
				continue;
			}

			auto& PointA = GetPointAt(x, y);
			auto& PointB = GetPointAt(x + 1, y);
			auto& PointC = GetPointAt(x, y + 1);
			auto& PointD = GetPointAt(x + 1, y + 1);

			Vector3 PointAPos = Vector3(x, PointA.Height, y);
			Vector3 PointBPos = Vector3(x + 1, PointB.Height, y);
			Vector3 PointCPos = Vector3(x, PointC.Height, y + 1);
			Vector3 PointDPos = Vector3(x + 1, PointD.Height, y + 1);

			// TRIANGLE A: PointA PointB PointC
			// TRIANGLE B: PointB PointC PointD

			Vector3 n = Vector3::Cross(PointCPos - PointAPos, PointBPos - PointAPos);
			if (x > X && y > Y)
			{
				PointA.Normal += n;
			}
			if (y > Y && x < W - 1)
			{
				PointB.Normal += n;
			}
			if (x > X && y < H - 1)
			{
				PointC.Normal += n;
			}

			n = Vector3::Cross(PointCPos - PointBPos, PointDPos - PointBPos);
			if (y > Y && x < W - 1)
			{
				PointB.Normal += n;
			}
			if (x > X && y < H - 1)
			{
				PointC.Normal += n;
			}
			if (x < W - 1 && y < H - 1)
			{
				PointD.Normal += n;
			}
		}
	}

	for (uint64 x = X; x < W; x++)
	{
		for (uint64 y = Y; y < H; y++)
		{
			if (x >= Width * LANDSCAPE_CHUNK_SIZE - 1 || y >= Height * LANDSCAPE_CHUNK_SIZE - 1)
			{
				continue;
			}

			if (x < 0 || y < 0)
			{
				continue;
			}

			auto& p = GetPointAt(x, y).Normal;
			p = p.Normalize();
		}
	}
}

std::vector<LandscapeChunk*> engine::LandscapeData::GetOverlappingChunks(BoundingBox Box)
{
	return std::vector<LandscapeChunk*>();
}

LandscapeChunk* engine::LandscapeData::CombineChunks(size_t X, size_t Y, size_t Size)
{
	if (Size == 1)
	{
		return new LandscapeChunk(GetChunk(X / LANDSCAPE_CHUNK_SIZE, Y / LANDSCAPE_CHUNK_SIZE));
	}

	LandscapeChunk* c = new LandscapeChunk();

	for (size_t iX = X; iX < X + Size * LANDSCAPE_CHUNK_SIZE; iX += Size)
	{
		for (size_t iY = Y; iY < Y + Size * LANDSCAPE_CHUNK_SIZE; iY += Size)
		{
			LandscapeChunk& Point = GetChunk(iX / LANDSCAPE_CHUNK_SIZE, iY / LANDSCAPE_CHUNK_SIZE);

			size_t RelativeX = iX % LANDSCAPE_CHUNK_SIZE;
			size_t RelativeY = iY % LANDSCAPE_CHUNK_SIZE;
			size_t TargetRelativeX = iX / Size % LANDSCAPE_CHUNK_SIZE;
			size_t TargetRelativeY = iY / Size % LANDSCAPE_CHUNK_SIZE;
			c->Points[TargetRelativeX + TargetRelativeY * LANDSCAPE_CHUNK_SIZE]
				= Point.Points[RelativeX + RelativeY * LANDSCAPE_CHUNK_SIZE];
		}
	}

	return c;
}

void engine::LandscapeData::SaveToStream(IBinaryStream* Stream)
{
	Stream->WriteString("hmp/0");
	Stream->WriteValue<uint64>(this->Width);
	Stream->WriteValue<uint64>(this->Height);

	mz_ulong BufferLength = this->Chunks.size() * sizeof(LandscapeChunk);

	uByte* Buffer = new uByte[BufferLength];

	auto Result = mz_compress(Buffer, &BufferLength, reinterpret_cast<uByte*>(this->Chunks.data()), BufferLength);

	if (Result != MZ_OK)
	{
		Log::Warn("Compress error");
	}

	Stream->WriteValue<uint64>(this->Chunks.size());
	Stream->WriteValue<uint64>(BufferLength);

	Stream->Write(Buffer, BufferLength);

	delete[] Buffer;
}

engine::LandscapeChunk::LandscapeChunk()
{
}

void engine::LandscapeChunk::Init(size_t X, size_t Y)
{
	for (size_t x = 0; x < LANDSCAPE_CHUNK_SIZE; x++)
	{
		for (size_t y = 0; y < LANDSCAPE_CHUNK_SIZE; y++)
		{
			Points[x + y * LANDSCAPE_CHUNK_SIZE].Height = (std::sin(float(x + X * LANDSCAPE_CHUNK_SIZE) / 20.0f) * std::sin(float(y + Y * LANDSCAPE_CHUNK_SIZE) / 20.0f)) * 10.0f;
		}
	}
}

void engine::LandscapePoint::Interpolate(const LandscapePoint& To, float Amount)
{
	Height = std::lerp(Height, To.Height, Amount);
	Normal = Normal + (To.Normal - Normal) * Amount;
}
