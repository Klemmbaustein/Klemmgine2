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

		RendererTexture* Draw(RendererTexture* Texture, PostProcess* With, Framebuffer* Buffer, Camera* Cam) override;
		ShaderObject* EffectShader = nullptr;
		void OnBufferResized(uint32 Width, uint32 Height) override;

	};
}