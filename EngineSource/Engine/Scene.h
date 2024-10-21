#pragma once
#include "Graphics/Drawable/IDrawable.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/Camera.h"
#include <kui/Rendering/Shader.h>

namespace engine
{
	class Scene
	{
	public:
		Scene();

		void Draw();
		void Update();

		graphics::Framebuffer* Buffer = nullptr;
		std::vector<graphics::IDrawable*> Drawables;
		graphics::Camera* Cam = nullptr;
	};
}