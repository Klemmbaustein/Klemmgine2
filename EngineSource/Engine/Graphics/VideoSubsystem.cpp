#include "VideoSubsystem.h"
#include <Engine/Subsystem/SceneSubsystem.h>
#include <Engine/Subsystem/ConsoleSubsystem.h>
#include <kui/App.h>
#include <SDL3/SDL.h>
#include <Engine/Internal/WMOptions.h>
#include <Engine/Engine.h>
#include <Engine/Input.h>
#include <Engine/Stats.h>
#include <Engine/UI/UICanvas.h>
#include <Core/LaunchArgs.h>
#include <Engine/Graphics/OpenGL.h>
#include <Engine/Debug/TimeLogger.h>
#include <stdexcept>
#include <kui/Rendering/OpenGLBackend.h>
#include <Engine/File/Resource.h>
#include <Engine/Graphics/Backend/OpenGLGraphicsBackend.h>

using namespace kui;
using namespace engine;
using namespace engine::subsystem;

engine::string VideoSubsystem::DefaultFontName = "res:DefaultFont.ttf";
VideoSubsystem* VideoSubsystem::Current = nullptr;

static float TimeScale = 1;

engine::VideoSubsystem::VideoSubsystem()
	: Subsystem("Video", Log::LogColor::Cyan)
{
	Current = this;
	app::error::SetErrorCallback([this](string Message, bool Fatal) {
		app::MessageBox("kui error: " + Message, "Error",
			Fatal ? app::MessageBoxType::Error : app::MessageBoxType::Warn);
		Print("kui error: " + Message, Fatal ? LogType::Critical : LogType::Error);
	});

	Print("Initializing SDL", LogType::Note);
	SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO);

	Print("Creating main window", LogType::Note);

	render::OpenGLBackend::UseAlphaBuffer = true;

	MainWindow = new Window("", Window::WindowFlag::Resizable
		| EngineWindowFlag::EngineWindow, Window::POSITION_CENTERED, GetWindowSize());
	MainWindow->SetMinSize(Vec2ui(600, 400));

	Backend = new graphics::OpenGLGraphicsBackend();
	Renderer = Backend->CreateRenderer();
	Textures.UsedRenderer = Renderer;
	Print(str::Format("Using graphics backend: %s", Backend->GetBackendIdentifier().c_str()), LogType::Note);

	MainWindow->SetTitle(GetWindowTitle().c_str());

	MainWindow->OnResizedCallback = [this](Window*) {
		OnResized();
	};

	static_cast<render::OpenGLBackend*>(MainWindow->UI.Render)->CanDrawToWindow = false;

	debug::TimeLogger ModulesTime{ "Compiled shader modules", GetLogPrefixes() };
	Shaders.Modules.ScanModules(Renderer);
	ModulesTime.End();

	MainWindow->Input.RegisterOnKeyDownCallback(Key::F11, [](Window* w) {
		w->SetMaximized(!w->GetWindowFullScreen());
	});

	string WindowsFontPath = "file:C:/Windows/Fonts/seguisb.ttf";
	if (resource::FileExists(WindowsFontPath))
	{
		DefaultFontName = WindowsFontPath;
	}

	if (!DefaultFont)
	{
		DefaultFont = new Font(DefaultFontName);
		MainWindow->Markup.SetDefaultFont(DefaultFont);
		MainWindow->Markup.AddFont("mono", DefaultFont);
	}
}

engine::string engine::VideoSubsystem::GetWindowTitle()
{
#define STR_INNER(x) # x
#define STR(x) STR_INNER(x)
	string Title = str::Format("Klemmgine 2 (%s, %s)", STR(ENGINE_COMPILER_ID), Backend->GetBackendIdentifier().c_str());

#ifdef EDITOR
	Title.append(" Editor");
#endif
	return Title;
}

kui::Vec2ui engine::VideoSubsystem::GetWindowSize()
{
	Vec2ui WindowSize = Window::SIZE_DEFAULT;

	auto SizeArg = launchArgs::GetArg("size");
	if (SizeArg && SizeArg->size() == 2)
	{
		WindowSize = Vec2ui(SizeArg->at(0).AsInt(), SizeArg->at(1).AsInt());

		if (WindowSize.X < 600 || WindowSize.Y < 400)
		{
			Print("Invalid window size, setting to default: " + WindowSize.ToString(), LogType::Warning);
			WindowSize = Window::SIZE_DEFAULT;
		}
	}

	return WindowSize;
}

void engine::VideoSubsystem::RegisterCommands(ConsoleSubsystem* System)
{
	System->AddCommand(console::Command{
		.Name = "reload_shaders",
		.Args = {},
		.OnCalled = [this](const console::Command::CallContext& ctx) {
			Print("Reloading shaders...");
			Shaders.ReloadAll();
		} });

	System->AddCommand(console::Command{
		.Name = "ui_scale",
		.Args = { console::Command::Argument{
			.Name = "scale",
			.Required = true,
		}, },
		.OnCalled = [this](const console::Command::CallContext& ctx) {
			try
			{
				MainWindow->DPIMultiplier = std::stof(ctx.ProvidedArguments.at(0));
			}
			catch (std::invalid_argument&)
			{
				ctx.Context->Print("Invalid ui scale: " + ctx.ProvidedArguments.at(0), LogType::Error);
			}
			catch (std::out_of_range&)
			{
				ctx.Context->Print("Invalid ui scale: " + ctx.ProvidedArguments.at(0), LogType::Error);
			}
		} });

	System->AddCommand(console::Command{
		.Name = "time_scale",
		.Args = { console::Command::Argument{
			.Name = "value",
			.Required = true,
		}, },
		.OnCalled = [this](const console::Command::CallContext& ctx) {
			try
			{
				TimeScale = std::stof(ctx.ProvidedArguments.at(0));
			}
			catch (std::invalid_argument&)
			{
				ctx.Context->Print("Invalid ui scale: " + ctx.ProvidedArguments.at(0), LogType::Error);
			}
			catch (std::out_of_range&)
			{
				ctx.Context->Print("Invalid ui scale: " + ctx.ProvidedArguments.at(0), LogType::Error);
			}
		} });

	System->AddCommand(console::Command{
		.Name = "vsync",
		.Args = { console::Command::Argument{
			.Name = "value",
			.Required = true,
		}, },
		.OnCalled = [this](const console::Command::CallContext& ctx) {
		auto& val = ctx.ProvidedArguments.at(0);

		if (val != "true" && val != "false")
		{
			ctx.Context->Print("Invalid vsync value", LogType::Error);
			return;
		}

		this->VSyncEnabled = val == "true";

		} });
}

engine::VideoSubsystem::~VideoSubsystem()
{
	Current = nullptr;
	GraphicsModel::ClearAll();
	graphics::CascadedShadows::UnloadShadows();
	delete Renderer;
	delete Backend;

	delete MainWindow;
}

void engine::VideoSubsystem::Update()
{
	MainWindow->Input.KeyboardFocusInput = input::ShowMouseCursor;
	if (!MainWindow->UpdateWindow())
	{
		Engine::Instance->ShouldQuit = true;
	}

	UICanvas::UpdateAll();

	input::IsLMBDown = MainWindow->Input.IsLMBDown;
	input::IsLMBClicked = MainWindow->Input.IsLMBClicked;
	input::IsRMBDown = MainWindow->Input.IsRMBDown;
	input::IsRMBClicked = MainWindow->Input.IsRMBClicked;

	stats::DeltaTime = MainWindow->GetDeltaTime() * TimeScale;
	stats::Time += stats::DeltaTime;
}

void engine::VideoSubsystem::RenderUpdate()
{
	SceneSubsystem* SceneSystem = Engine::GetSubsystem<SceneSubsystem>();

	if (SceneSystem)
	{
		for (Scene* scn : SceneSystem->LoadedScenes)
		{
			scn->Graphics.Draw(Renderer);
		}
	}

	graphics::RendererTexture* Texture = SceneSystem && SceneSystem->Main
		? SceneSystem->Main->Graphics.GetDrawBuffer() : nullptr;

	Renderer->RenderScreen(MainWindow, Texture, VSyncEnabled);
}

void engine::VideoSubsystem::OnResized()
{
	OnResizedCallbacks.Invoke(MainWindow->GetSize());
}