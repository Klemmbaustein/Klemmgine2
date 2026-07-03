#pragma once
#include <Core/Types.h>
#include <Engine/Graphics/Camera.h>
#include <Engine/Graphics/Backend/Renderer.h>

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
		Renderer* Render = nullptr;

		virtual void OnBufferResized(uint32 Width, uint32 Height);
		virtual RendererTexture* Draw(RendererTexture* Texture, PostProcess* With, Framebuffer* Buffer, Camera* Cam) = 0;


	protected:

		DrawCommand* StartRenderPass();

		// Framebuffer utility functions
		RendererDrawTarget* CreateNewBuffer(uint32 Width, uint32 Height, bool FilterNearest);

		RendererTexture* DrawBuffer(DrawCommand* Pass, RendererDrawTarget* Target, uint32 Width, uint32 Height);

	};
}