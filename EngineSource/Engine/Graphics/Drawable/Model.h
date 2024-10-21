#pragma once
#include "IDrawable.h"
#include <Engine/Graphics/VertexBuffer.h>
#include <Engine/Graphics/ShaderObject.h>

namespace engine::graphics
{
	class Model : public IDrawable
	{
	public:
		Model();
		~Model();

		ShaderObject* ModelShader;
		VertexBuffer* ModelVertexBuffer;

		virtual void Draw(Camera* With) override;
	};
}