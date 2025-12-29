#include "VideoSubsystem.h"
#include <Engine/Subsystem/SceneSubsystem.h>
#include <Engine/Subsystem/ConsoleSubsystem.h>
#include <kui/App.h>
#include <SDL3/SDL.h>
#include <Engine/Internal/WMOptions.h>
#include <Engine/Internal/OpenGL.h>
#include <Engine/Internal/SystemWM_SDL3.h>
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

#ifdef EDITOR
#include <Editor/EditorSubsystem.h>
#include <Editor/UI/Panels/Viewport.h>
#include <Editor/Editor.h>
#endif

using namespace kui;
using namespace engine;
using namespace engine::subsystem;

static void GLAPIENTRY MessageCallback(
	GLenum source, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar* message, const void* userParam)
{
	if (type == GL_DEBUG_TYPE_ERROR
		|| type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR
		|| type == GL_DEBUG_TYPE_PORTABILITY)
	{
		((VideoSubsystem*)userParam)->Print(message, Subsystem::LogType::Error);
	}
}

static systemWM::SysWindow* GetSysWindow(kui::Window* From)
{
	return static_cast<systemWM::SysWindow*>(From->GetSysWindow());
}

engine::string VideoSubsystem::DefaultFontName = "res:DefaultFont.ttf";
VideoSubsystem* VideoSubsystem::Current = nullptr;

static float TimeScale = 1;

engine::VideoSubsystem::VideoSubsystem()
	: Subsystem("Video", Log::LogColor::Cyan)
{
	Current = this;
	app::error::SetErrorCallback([this](string Message, bool Fatal)
	{
		app::MessageBox("kui error: " + Message, "Error",
			Fatal ? app::MessageBoxType::Error : app::MessageBoxType::Warn);
		Print("kui error: " + Message, Fatal ? LogType::Critical : LogType::Error);
	});

	Print("Initializing SDL", LogType::Note);
	SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO);

	Print("Creating main window", LogType::Note);

	render::OpenGLBackend::UseAlphaBuffer = true;

	auto VersionArg = launchArgs::GetArg("gl");

	if (VersionArg.has_value() && VersionArg->size() == 1)
	{
		string VersionString = VersionArg->at(0).AsString();

		if (VersionString == "4.3")
		{
			openGL::VersionOverride = openGL::Version::GL430;
		}
		else if (VersionString == "3.3")
		{
			openGL::VersionOverride = openGL::Version::GL330;
		}
		else
		{
			Print(str::Format("Unknown OpenGL version '%s' will be interpreted as default",
				VersionString.c_str()));
		}

		Print(str::Format("OpenGL version set through command line: '%s'", VersionString.c_str()));
	}

	MainWindow = new Window(GetWindowTitle(), Window::WindowFlag::Resizable
		| EngineWindowFlag::EngineWindow, Window::POSITION_CENTERED, GetWindowSize());
	MainWindow->SetMinSize(Vec2ui(600, 400));

	if (openGL::GetGLVersion() < openGL::Version::GL430)
	{
		Print("Using OpenGL 3.3 instead of 4.3. Some graphics effects might not work.", LogType::Warning);
	}

	MainWindow->OnResizedCallback = [this](Window*)
	{
		OnResized();
	};

	static_cast<render::OpenGLBackend*>(MainWindow->UI.Render)->CanDrawToWindow = false;

	InitGLErrors();

	debug::TimeLogger ModulesTime{ "Compiled shader modules", GetLogPrefixes() };
	Shaders.Modules.ScanModules();
	ModulesTime.End();

	MainWindow->Input.RegisterOnKeyDownCallback(Key::F11, [](Window* w)
	{
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
	string Title = str::Format("Klemmgine 2 (%s, OpenGL %s)", STR(ENGINE_COMPILER_ID), "4.3+");

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
	delete MainWindow;
}

void engine::VideoSubsystem::Update()
{
	if (!MainWindow->UpdateWindow())
	{
		Engine::Instance->ShouldQuit = true;
	}

	if (Engine::IsPlaying)
	{
		UICanvas::UpdateAll();
	}

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
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glClearColor(0, 0, 0, 1);
		for (Scene* scn : SceneSystem->LoadedScenes)
		{
			scn->Draw();
		}
		glViewport(0, 0, GLsizei(MainWindow->GetSize().X), GLsizei(MainWindow->GetSize().Y));
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
	}

	auto PostProcess = MainWindow->Shaders.LoadShader("shader/internal/postProcess.vert",
		"shader/internal/drawToWindow.frag", "engineToWindow");

	uint32 Texture = 0;

	if (SceneSystem && SceneSystem->Main)
	{
		Texture = SceneSystem->Main->GetDrawBuffer();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	PostProcess->Bind();
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, Texture);

	auto GlRender = static_cast<render::OpenGLBackend*>(MainWindow->UI.Render);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, GlRender->UITextures[0]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, GlRender->UITextures[1]);

	PostProcess->SetInt("u_texture", 0);
	PostProcess->SetInt("u_ui", 1);
	PostProcess->SetInt("u_alpha", 2);

#if EDITOR
	if (EditorSubsystem::Active)
	{
		editor::Viewport* View = editor::Viewport::Current;

		PostProcess->SetVec2("u_pos", View->ViewportBackground->GetScreenPosition() / 2 + 0.5);
		PostProcess->SetVec2("u_size", View->ViewportBackground->GetUsedSize().GetScreen() / 2);
	}
	else
#endif
	{
		PostProcess->SetVec2("u_pos", 0);
		PostProcess->SetVec2("u_size", 1);
	}

	glDrawArrays(GL_TRIANGLES, 0, 3);
	SDL_GL_SetSwapInterval(this->VSyncEnabled ? 1 : 0);
	SDL_GL_SwapWindow(GetSysWindow(MainWindow)->SDLWindow);
}

void engine::VideoSubsystem::OnResized()
{
	for (auto& [_, Callback] : OnResizedCallbacks)
	{
		Callback(MainWindow->GetSize());
	}
}

void engine::VideoSubsystem::InitGLErrors()
{
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, this);
}
