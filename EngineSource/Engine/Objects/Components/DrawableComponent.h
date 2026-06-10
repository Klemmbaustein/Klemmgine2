#pragma once
#include "ObjectComponent.h"
#include <Core/BoundingBox.h>
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
		BoundingBox DrawBoundingBox;
		bool DrawStencil = false;
		bool IsVisible = true;
		bool CastShadow = true;
		bool IsOpaque = true;
		bool IsTransparent = false;
		virtual void Draw(graphics::Camera* From, graphics::GraphicsScene* In) = 0;
		virtual void DrawTransparent(graphics::Camera* From, graphics::GraphicsScene* In)
		{

		}
		virtual void SimpleDraw(graphics::ShaderObject* With) {};

	private:
		static inline uint64 IdCounter = 0;
	public:

		const uint64 UniqueId = IdCounter++;
	};
}