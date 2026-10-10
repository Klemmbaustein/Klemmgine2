#pragma once
#include <Engine/Graphics/Backend/Renderer.h>
#include <Engine/Graphics/ShaderObject.h>
#include <Engine/Graphics/Camera.h>
#include <Engine/Graphics/Material.h>
#include <Core/BoundingBox.h>
#include <Core/Transform.h>

namespace engine
{
	struct ModelData;
}

namespace engine::graphics
{
	class GraphicsScene;

	class Model
	{
	public:
		Model(const ModelData* From);
		~Model();

		std::vector<VertexBuffer*> ModelVertexBuffers;

		void Draw(Renderer* Render, GraphicsScene* In, const Transform& At, Camera* With,
			std::vector<Material*>& UsedMaterials, const BoundingBox& Bounds, bool Stencil, bool IsTransparent);
		void DrawInstanced(Renderer* Render, GraphicsScene* In, const Transform& At, Camera* With,
			std::vector<Material*>& UsedMaterials, const BoundingBox& Bounds, bool Stencil, bool IsTransparent,
			size_t Count);
		void SimpleDraw(Renderer* Render, const Transform& At, ShaderObject* Shader,
			std::vector<Material*>& UsedMaterials);
		void SimpleDrawInstanced(Renderer* Render, const Transform& At, ShaderObject* Shader,
			std::vector<Material*>& UsedMaterials, size_t Count);
	};
}