#pragma once
#include "Engine/Types.h"

namespace engine::graphics
{
	class Framebuffer
	{
	public:
		uint32 Buffer = 0, Texture = 0;
		int64 Width = 0, Height = 0;

		Framebuffer(int64 Width, int64 Height);
		~Framebuffer();

		void Resize(int64 NewWidth, int64 NewHeight);

		void Bind() const;
		void Unbind() const;
	};
}