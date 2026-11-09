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

		void Update() override;

		// Inherited via DrawableComponent
		void Draw(graphics::Renderer* Render, graphics::Camera* From, graphics::GraphicsScene* In) override;

		void Load(AssetRef File);
		std::shared_ptr<graphics::ParticleData> Data;

		void InitializeEmitter(graphics::ParticleEmitter* Emitter);
		void RemoveEmitter(size_t Index);

		void Reset() const;
		bool IsPlaying() const;
		bool DoHotReload = true;

	private:
		void Clear();

		size_t InstanceCount = 0;
	};
}