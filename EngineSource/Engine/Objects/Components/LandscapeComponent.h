#pragma once
#include "DrawableComponent.h"
#include <Engine/Graphics/VertexBuffer.h>
#include <Engine/Graphics/Material.h>
#include <Engine/Physics/Physics.h>

namespace engine
{
	class LandscapeComponent : public DrawableComponent
	{
	public:

		physics::PhysicsBody* Collider = nullptr;

		void OnAttached() override;
		void OnDetached() override;

		void Draw(graphics::Camera* From, graphics::GraphicsScene* In) override;
		void SimpleDraw(graphics::ShaderObject* With) override;

		void UpdateTransform(bool Dirty) override;

		graphics::Material* LandscapeMaterial = nullptr;

	private:
		graphics::VertexBuffer* LandscapeMesh = nullptr;
	};
}