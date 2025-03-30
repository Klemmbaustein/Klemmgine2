#pragma once
#include "ObjectComponent.h"
#include <Engine/Graphics/BoundingBox.h>
#include <Engine/Graphics/ShaderObject.h>

namespace engine
{
	class DrawableComponent : public ObjectComponent
	{
	public:
		graphics::BoundingBox DrawBoundingBox;
		bool IsVisible = true;
		bool CastShadow = true;
		virtual void Draw(graphics::Camera* From) = 0;
		virtual void SimpleDraw(graphics::ShaderObject* With) {};
	};
}