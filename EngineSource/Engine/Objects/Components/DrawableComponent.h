#pragma once
#include "ObjectComponent.h"
#include <Engine/Graphics/BoundingBox.h>

namespace engine
{
	class DrawableComponent : public ObjectComponent
	{
	public:
		graphics::BoundingBox DrawBoundingBox;
		bool IsVisible = true;
		virtual void Draw(graphics::Camera* From) = 0;
	};
}