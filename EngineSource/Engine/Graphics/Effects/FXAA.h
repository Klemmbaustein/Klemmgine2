#pragma once
#include <Engine/Graphics/Effects/PostProcessEffect.h>
#include <Engine/Graphics/ShaderObject.h>

namespace engine::graphics
{
	class FXAA : public PostProcessEffect
	{
	public:

		FXAA();

		~FXAA() override;

		static constexpr int64 AA_DRAW_ORDER = 100;

		uint32 Draw(uint32 Texture, graphics::PostProcess* With, graphics::Framebuffer* Buffer, graphics::Camera* Cam) override;
		ShaderObject* EffectShader = nullptr;
		void OnBufferResized(uint32 Width, uint32 Height) override;

	};
}