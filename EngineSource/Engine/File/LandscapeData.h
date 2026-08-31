#pragma once
#include <Core/Vector.h>
#include <array>
#include <Core/BoundingBox.h>
#include <Core/File/BinaryStream.h>
#include <Core/Types.h>
#include <Engine/File/AssetRef.h>
#include <mutex>
#include <memory>
#include <Engine/Physics/Physics.h>

namespace engine
{
	constexpr inline size_t LANDSCAPE_CHUNK_SIZE = 16;

	class ObjectComponent;

	class LandscapePoint
	{
	public:
		float Height = 0.0f;
		Vector3 Color = 1.0f;
		float Opacity = 1.0f;
		Vector3 Normal = Vector3(0, 1, 0);

		void Interpolate(const LandscapePoint& To, float Amount);
	};

	class LandscapeChunk
	{
	public:
		LandscapeChunk();

		std::array<LandscapePoint, LANDSCAPE_CHUNK_SIZE * LANDSCAPE_CHUNK_SIZE> Points;

		void Init(size_t X, size_t Y);
	};

	class LandscapeData
	{
	public:

		LandscapeData(size_t Width, size_t Height);
		LandscapeData(AssetRef Asset);
		LandscapeData();
		~LandscapeData();

		void InitializeNormal();
		void LoadCollider(ObjectComponent* Component);
		void InitializeNormalForRange(int64 X, int64 Y, int64 W, int64 H);

		std::vector<LandscapeChunk*> GetOverlappingChunks(BoundingBox Box);

		LandscapeChunk* CombineChunks(size_t X, size_t Y, size_t Size);

		inline LandscapeChunk& GetChunk(size_t X, size_t Y)
		{
			return Chunks[Y * Width + X];
		}

		LandscapePoint& GetPointAt(size_t X, size_t Y)
		{
			return GetChunk(X / LANDSCAPE_CHUNK_SIZE, Y / LANDSCAPE_CHUNK_SIZE)
				.Points[X % LANDSCAPE_CHUNK_SIZE + (Y % LANDSCAPE_CHUNK_SIZE * LANDSCAPE_CHUNK_SIZE)];
		}

		std::vector<LandscapeChunk> Chunks;
		size_t Width = 0;
		size_t Height = 0;

		std::mutex Lock;
		physics::HeightMapBody* Collider = nullptr;

		bool Changed = false;

		void SaveToStream(IBinaryStream* Stream);
	};

	struct LandscapeDataRef
	{
		std::shared_ptr<LandscapeData> Ptr;
	};
}