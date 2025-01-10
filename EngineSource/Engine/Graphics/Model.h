#pragma once
#include <Engine/Graphics/VertexBuffer.h>
#include <Engine/Graphics/ShaderObject.h>
#include <Engine/Graphics/Camera.h>
#include <Engine/Graphics/Material.h>
#include <Engine/Transform.h>

namespace engine
{
	struct ModelData;
}

namespace engine::graphics
{
	class Model
	{
	public:
		Model(const ModelData* From);
		~Model();

		std::vector<VertexBuffer*> ModelVertexBuffers;

		virtual void Draw(const Transform& At, Camera* With, std::vector<Material*>& UsedMaterials);
		virtual void SimpleDraw(const Transform& At, ShaderObject* Shader);
	};
}