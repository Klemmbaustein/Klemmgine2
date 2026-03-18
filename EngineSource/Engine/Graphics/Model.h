#pragma once
#include <Engine/Graphics/VertexBuffer.h>
#include <Engine/Graphics/ShaderObject.h>
#include <Engine/Graphics/Camera.h>
#include <Engine/Graphics/Material.h>
#include <Engine/Graphics/BoundingBox.h>
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

		virtual void Draw(GraphicsScene* In, const Transform& At, Camera* With,
			std::vector<Material*>& UsedMaterials, const BoundingBox& Bounds, bool Stencil);
		virtual void SimpleDraw(const Transform& At, ShaderObject* Shader,
			std::vector<Material*>& UsedMaterials);
	};
}