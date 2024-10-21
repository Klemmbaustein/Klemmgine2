#pragma once
#include <Engine/Graphics/Camera.h>

namespace engine::graphics
{
	class IDrawable
	{
	public:
		IDrawable();
		virtual ~IDrawable();

		virtual void Draw(Camera* With) = 0;
	};
}