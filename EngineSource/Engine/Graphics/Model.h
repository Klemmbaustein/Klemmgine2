#pragma once
#include <Engine/Graphics/VertexBuffer.h>
#include <Engine/Graphics/ShaderObject.h>
#include <Engine/Graphics/Camera.h>
#include <Engine/Graphics/Material.h>

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

		virtual void Draw(Vector3 At, Camera* With, std::vector<Material*>& UsedMaterials);
	};
}