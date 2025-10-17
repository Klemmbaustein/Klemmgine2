#pragma once
#include <Engine/Graphics/VertexBuffer.h>
#include <Engine/Graphics/ShaderObject.h>
#include <Engine/Graphics/Camera.h>
#include <Engine/Graphics/Material.h>
#include <Core/Transform.h>

namespace engine
{
	struct ModelData;
	class Scene;
}

namespace engine::graphics
{
	class Model
	{
	public:
		Model(const ModelData* From);
		~Model();

		std::vector<VertexBuffer*> ModelVertexBuffers;

		virtual void Draw(Scene* In, const Transform& At, Camera* With,
			std::vector<Material*>& UsedMaterials, bool Stencil);
		virtual void SimpleDraw(const Transform& At, ShaderObject* Shader,
			std::vector<Material*>& UsedMaterials);
	};
}