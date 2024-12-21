#pragma once
#include "ISubsystem.h"
#include <kui/Window.h>
#include <Engine/Graphics/ShaderLoader.h>
#include <Engine/Graphics/Texture.h>
#include <map>

namespace engine::subsystem
{
	class VideoSubsystem : public ISubsystem
	{
	public:
		VideoSubsystem();
		virtual ~VideoSubsystem() override;

		virtual void Update() override;
		virtual void RenderUpdate() override;

		kui::Window* MainWindow = nullptr;
		graphics::ShaderLoader Shaders;
		graphics::TextureLoader Textures;

		std::map<void*, std::function<void(kui::Vec2ui NewSize)>> OnResizedCallbacks;

	private:
		void OnResized();
	};
}