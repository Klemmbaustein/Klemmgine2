#pragma once
#include "Engine/Types.h"

namespace engine::graphics
{
	class Framebuffer
	{
	public:
		uint32 Buffer = 0, Texture = 0;

		Framebuffer();
		~Framebuffer();

		void Resize(int64 NewWidth, int64 NewHeight);

		void Bind() const;
		void Unbind() const;
	};
}