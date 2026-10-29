#pragma once
#include <Engine/Objects/Components/DrawableComponent.h>
#include <memory>
#include <Engine/Graphics/ParticleData.h>

namespace engine
{
	class ParticleComponent : public DrawableComponent
	{
	public:
		ParticleComponent() = default;
		ParticleComponent(const ParticleComponent&) = delete;
		~ParticleComponent() override;

		void OnAttached() override;
		void OnDetached() override;
		bool UpdateTransform(bool IsDirty) override;

		void Update() override;

		// Inherited via DrawableComponent
		void Draw(graphics::Renderer* Render, graphics::Camera* From, graphics::GraphicsScene* In) override;

		void Load(AssetRef File);
		std::shared_ptr<graphics::ParticleData> Data;

	private:
		void Clear();

		size_t InstanceCount = 0;
		BoundingBox EmitterBounds;
	};
}