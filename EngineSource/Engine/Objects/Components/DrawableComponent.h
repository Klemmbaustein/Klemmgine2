#pragma once
#include "ObjectComponent.h"
#include <Engine/Graphics/BoundingBox.h>
#include <Engine/Graphics/Camera.h>
#include <Engine/Graphics/ShaderObject.h>

namespace engine::graphics
{
	class GraphicsScene;
}

namespace engine
{
	class DrawableComponent : public ObjectComponent
	{
	public:
		graphics::BoundingBox DrawBoundingBox;
		bool DrawStencil = false;
		bool IsVisible = true;
		bool CastShadow = true;
		bool IsTransparent = false;
		virtual void Draw(graphics::Camera* From, graphics::GraphicsScene* In) = 0;
		virtual void SimpleDraw(graphics::ShaderObject* With) {};

	private:
		static inline uint64 IdCounter = 0;
	public:

		const uint64 UniqueId = IdCounter++;
	};
}