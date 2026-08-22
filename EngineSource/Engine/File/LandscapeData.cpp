#include "LandscapeData.h"

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

	for (size_t x = 0; x < Width * LANDSCAPE_CHUNK_SIZE - 1; x++)
	{
		for (size_t y = 0; y < Height * LANDSCAPE_CHUNK_SIZE - 1; y++)
		{
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
			PointA.Normal += n;
			PointB.Normal += n;
			PointC.Normal += n;

			n = Vector3::Cross(PointCPos - PointBPos, PointDPos - PointBPos);
			PointB.Normal += n;
			PointC.Normal += n;
			PointD.Normal += n;
		}
	}

	for (size_t x = 0; x < Width * LANDSCAPE_CHUNK_SIZE; x++)
	{
		for (size_t y = 0; y < Height * LANDSCAPE_CHUNK_SIZE; y++)
		{
			auto& p = GetPointAt(x, y).Normal;
			p = p.Normalize();
		}
	}

}

engine::LandscapeData::LandscapeData()
{
}

engine::LandscapeData::~LandscapeData()
{
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
}
