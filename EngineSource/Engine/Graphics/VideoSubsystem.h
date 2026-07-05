#pragma once
#include <Engine/Subsystem/Subsystem.h>
#include <kui/Window.h>
#include <Engine/Graphics/ShaderLoader.h>
#include <Engine/Graphics/Texture.h>
#include <Engine/Graphics/Backend/GraphicsBackend.h>
#include <map>
#include <Core/Event.h>

namespace engine
{
	class VideoSubsystem : public subsystem::Subsystem
	{
	public:
		VideoSubsystem();
		virtual ~VideoSubsystem() override;

		virtual void Update() override;
		virtual void RenderUpdate() override;

		virtual void RegisterCommands(ConsoleSubsystem* System) override;

		kui::Window* MainWindow = nullptr;
		kui::Font* DefaultFont = nullptr;
		graphics::ShaderLoader Shaders;
		graphics::TextureLoader Textures;
		graphics::GraphicsBackend* Backend = nullptr;
		graphics::Renderer* Renderer = nullptr;

		bool VSyncEnabled = true;
		bool DrawShadows = true;
		bool DrawAmbientOcclusion = true;

		Event<kui::Vec2f> OnResizedCallbacks;
		static string DefaultFontName;
		static VideoSubsystem* Current;

		void OnResized();

	private:
		string GetWindowTitle();
		kui::Vec2ui GetWindowSize();
	};
}