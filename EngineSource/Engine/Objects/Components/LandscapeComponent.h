#pragma once
#include "DrawableComponent.h"
#include <Engine/Graphics/Backend/Renderer.h>
#include <Engine/Graphics/Material.h>
#include <Engine/Physics/Physics.h>
#include <memory>
#include <Engine/File/LandscapeData.h>

namespace engine
{
	class LandscapeMeshGenerator;

	struct LandscapeSegment
	{
		LandscapeSegment(size_t X, size_t Y, size_t Scale);
		LandscapeSegment(LandscapeChunk* PlaceholderChunk, size_t X, size_t Y, size_t Scale);

		~LandscapeSegment()
		{
			if (SegmentMesh)
			{
				delete SegmentMesh;
			}
			else
			{
				for (auto& i : SubSegments)
				{
					delete i;
				}
			}
		}

		graphics::VertexBuffer* SegmentMesh = nullptr;
		LandscapeSegment* SubSegments[4]{};

		LandscapeChunk* PlaceholderChunk = nullptr;

		std::vector<graphics::Vertex> Vertices;
		std::vector<uint32> Indices;

		BoundingBox Bounds;

		size_t ChunkX = 0, ChunkY = 0, Scale = 0;
		size_t LeftScaleDifference = 1, BottomScaleDifference = 1;

		void BuildBuffer(LandscapeSegment* Chunks[4], LandscapeMeshGenerator* Generator);

		void Draw(graphics::DrawCommand* Pass, graphics::ShaderObject* WithShader);
		void Draw(graphics::DrawCommand* Pass, graphics::ShaderObject* WithShader, graphics::Camera* Cam);

		void CreateVertexBuffer(const Transform& WithTransform);
	};

	class LandscapeMeshGenerator
	{
	public:
		LandscapeSegment* RootSegment = nullptr;
		std::shared_ptr<LandscapeData> Data = nullptr;
		bool IsDone = false;
		float LodFalloff = 1.5f;
		float UvScale = 1.0f;
		Vector3 CameraPosition;

		LandscapeSegment* BuildSegment(size_t X, size_t Y, size_t Scale);
		void MergeSegments(LandscapeSegment* From, LandscapeSegment* Left, LandscapeSegment* Bottom);
		void BuildSegments(LandscapeSegment* From, size_t X, size_t Y, size_t Scale);
		LandscapeSegment* GetSegment(LandscapeSegment* From, size_t ChunkX, size_t ChunkY);

		size_t CalculateLodScale(size_t X, size_t Y, size_t Scale) const;
	};

	class LandscapeComponent : public DrawableComponent
	{
	public:

		physics::PhysicsBody* Collider = nullptr;

		void OnAttached() override;
		void OnDetached() override;

		void Load(AssetRef HeightmapFile);

		void Draw(graphics::Renderer* Render, graphics::Camera* From, graphics::GraphicsScene* In) override;
		void SimpleDraw(graphics::Renderer* Render, graphics::ShaderObject* With) override;

		void Update() override;

		bool UpdateTransform(bool Dirty) override;

		graphics::Material* LandscapeMaterial = nullptr;
		std::shared_ptr<LandscapeData> Data = nullptr;
		std::shared_ptr<LandscapeMeshGenerator> Generator = nullptr;

		LandscapeSegment* RootSegment = nullptr;

		void Generate();

		bool CanGenerate = false;
		bool IsDirty = false;

		float LodFalloff = 1.5f;
		float UvScale = 1.0f;

		bool CheckLod(LandscapeSegment* ForSegment);

	private:

		void InitializeMesh();

		Vector3 LastScale = 1.0f;
	};
}