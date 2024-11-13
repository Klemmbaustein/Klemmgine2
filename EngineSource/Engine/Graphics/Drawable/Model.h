#pragma once
#include "IDrawable.h"
#include <Engine/Graphics/VertexBuffer.h>
#include <Engine/Graphics/ShaderObject.h>
#include <Engine/File/ModelData.h>

namespace engine::graphics
{
	class Model : public IDrawable
	{
	public:
		Model(ModelData* From);
		~Model();

		ShaderObject* ModelShader;
		std::vector<VertexBuffer*> ModelVertexBuffer;

		virtual void Draw(Camera* With) override;
	};
}