#pragma once
#include "Core/Types.h"

namespace engine::graphics
{
	class Framebuffer
	{
	public:
		static constexpr size_t NUM_TEXTURES = 4;

		static constexpr size_t TEXTURE_COLOR = 0;
		static constexpr size_t TEXTURE_DEPTH_STENCIL = 1;
		static constexpr size_t TEXTURE_POSITION = 2;
		static constexpr size_t TEXTURE_NORMAL = 3;

		uint32 Buffer = 0;
		uint32 Textures[NUM_TEXTURES];
		int64 Width = 0, Height = 0;

		static const uint32 DRAW_ATTACHMENTS[3];

		Framebuffer(int64 Width, int64 Height);
		~Framebuffer();

		void Resize(int64 NewWidth, int64 NewHeight);

		void Bind() const;
		void Unbind() const;
	};
}