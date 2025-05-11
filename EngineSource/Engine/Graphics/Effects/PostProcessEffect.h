#pragma once
#include <Core/Types.h>
#include <Engine/Graphics/Camera.h>

namespace engine::graphics
{
	class PostProcess;
	class Framebuffer;

	class PostProcessEffect
	{
	public:
		PostProcessEffect();
		virtual ~PostProcessEffect();

		// Might as well make it 64 bits because of struct padding.
		int64 DrawOrder = 0;

		virtual void OnBufferResized(uint32 Width, uint32 Height);
		virtual uint32 Draw(uint32 Texture, PostProcess* With, Framebuffer* Buffer, Camera* Cam) = 0;

	protected:

		// Framebuffer utility functions
		uint64 CreateNewBuffer(uint32 Width, uint32 Height, bool FilterNearest);

		uint32 DrawBuffer(uint64 Buffer, uint32 Width, uint32 Height);
		void FreeBuffer(uint64 Buffer);

	};
}