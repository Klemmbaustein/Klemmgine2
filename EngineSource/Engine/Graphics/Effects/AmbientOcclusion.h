#pragma once
#include "PostProcessEffect.h"
#include <Core/Vector.h>
#include <Engine/Graphics/ShaderObject.h>

namespace engine::graphics
{
	class AmbientOcclusion : public PostProcessEffect
	{
	public:
		AmbientOcclusion();

		uint32 Draw(uint32 Texture, PostProcess* With, Framebuffer* Buffer, Camera* Cam) override;
		void OnBufferResized(uint32 Width, uint32 Height) override;
		static constexpr int64 AO_DRAW_ORDER = 5;

	private:

		uint64 AoBuffer = 0;

		uint32 AoWidth = 0, AoHeight = 0;
		uint32 Width = 0, Height = 0;

		std::vector<Vector3> AoKernel;
		ShaderObject* AoShader = nullptr;
		ShaderObject* AoMergeShader = nullptr;
		uint32 NoiseTexture = 0;
	};
}