#pragma once
#include "PostProcessEffect.h"
#include <Engine/Graphics/ShaderObject.h>

namespace engine::graphics
{
	class Bloom : public PostProcessEffect
	{
	public:
		Bloom();
		~Bloom() override;

		static constexpr int64 BLOOM_DRAW_ORDER = 20;

		uint32 Draw(uint32 Texture, PostProcess* With, Framebuffer* Buffer, Camera* Cam) override;
		void OnBufferResized(uint32 Width, uint32 Height) override;

	private:

		ShaderObject* BloomShader = nullptr;
		ShaderObject* BloomMergeShader = nullptr;

		uint32 BloomWidth = 0;
		uint32 BloomHeight = 0;
		uint32 Width = 0;
		uint32 Height = 0;
		uint32 BloomTextureLocation = 0, TextureSizeLocation = 0;
		uint64 BloomBuffers[2];
	};
}