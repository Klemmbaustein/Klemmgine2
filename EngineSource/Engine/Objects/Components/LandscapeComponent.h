#pragma once
#include "DrawableComponent.h"
#include <Engine/Graphics/Backend/Renderer.h>
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

		void Draw(graphics::Renderer* Render, graphics::Camera* From, graphics::GraphicsScene* In) override;
		void SimpleDraw(graphics::Renderer* Render, graphics::ShaderObject* With) override;

		bool UpdateTransform(bool Dirty) override;

		graphics::Material* LandscapeMaterial = nullptr;

	private:
		graphics::VertexBuffer* LandscapeMesh = nullptr;
	};
}