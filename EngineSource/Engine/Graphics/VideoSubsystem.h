#pragma once
#include <Engine/Subsystem/Subsystem.h>
#include <kui/Window.h>
#include <Engine/Graphics/ShaderLoader.h>
#include <Engine/Graphics/Texture.h>
#include <map>

namespace engine
{
	class VideoSubsystem : public subsystem::Subsystem
	{
	public:
		VideoSubsystem();
		virtual ~VideoSubsystem() override;

		virtual void Update() override;
		virtual void RenderUpdate() override;

		virtual void RegisterCommands(subsystem::ConsoleSubsystem* System) override;

		kui::Window* MainWindow = nullptr;
		kui::Font* DefaultFont = nullptr;
		graphics::ShaderLoader Shaders;
		graphics::TextureLoader Textures;

		bool VSyncEnabled = true;

		std::map<void*, std::function<void(kui::Vec2ui NewSize)>> OnResizedCallbacks;
		static string DefaultFontName;
		static VideoSubsystem* Current;

		void OnResized();

		void InitGLErrors();

	private:
		string GetWindowTitle();
		kui::Vec2ui GetWindowSize();
	};
}