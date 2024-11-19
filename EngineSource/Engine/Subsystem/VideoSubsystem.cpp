#include "VideoSubsystem.h"
#include "SceneSubsystem.h"
#include <kui/App.h>
#include <SDL3/SDL.h>
#include <GL/glew.h>
#include "EditorSubsystem.h"
#include <Engine/Internal/SystemWM_SDL3.h>
#include <Engine/Engine.h>
#include <Engine/Input.h>
#include <Engine/Stats.h>
#include <Engine/Editor/UI/Viewport.h>
#include <iostream>
using namespace kui;

static void GLAPIENTRY MessageCallback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam
)
{
	if (type == GL_DEBUG_TYPE_ERROR
		|| type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR
		|| type == GL_DEBUG_TYPE_PORTABILITY)
	{
		((engine::subsystem::VideoSubsystem*)userParam)->Print(std::string(message), engine::subsystem::ISubsystem::LogType::Error);
	}
}

static systemWM::SysWindow* GetSysWindow(kui::Window* From)
{
	return static_cast<systemWM::SysWindow*>(From->GetSysWindow());
}

engine::subsystem::VideoSubsystem::VideoSubsystem()
	: ISubsystem("Video", Log::LogColor::Cyan)
{
	app::error::SetErrorCallback([this](string Message, bool Fatal)
		{
			Print("kui error: " + Message, Fatal ? LogType::Critical : LogType::Error);
		});

	Print("Initializing SDL", LogType::Info);
	SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO);

	Print("Creating main window", LogType::Info);

	UIManager::UseAlphaBuffer = true;

	MainWindow = new Window("Klemmgine 2", Window::WindowFlag::Resizable);

	MainWindow->OnResizedCallback = [this](Window*)
		{
			Print("Main window resized", LogType::Note);
			OnResized();
		};

	MainWindow->UI.DrawToWindow = false;
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, this);
}

engine::subsystem::VideoSubsystem::~VideoSubsystem()
{
	delete MainWindow;
}

void engine::subsystem::VideoSubsystem::Update()
{
	if (!MainWindow->UpdateWindow())
	{
		Engine::Instance->ShouldQuit = true;
	}
	input::IsLMBDown = MainWindow->Input.IsLMBDown;
	input::IsLMBClicked = MainWindow->Input.IsLMBClicked;
	input::IsRMBDown = MainWindow->Input.IsRMBDown;
	input::IsRMBClicked = MainWindow->Input.IsRMBClicked;

	stats::DeltaTime = MainWindow->GetDeltaTime();
	stats::Time += stats::DeltaTime;
}

void engine::subsystem::VideoSubsystem::RenderUpdate()
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
		glViewport(0, 0, MainWindow->GetSize().X, MainWindow->GetSize().Y);
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
	}

	auto PostProcess = MainWindow->Shaders.LoadShader("shader/postProcess.vert", "shader/drawToWindow.frag", "engineToWindow");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	PostProcess->Bind();
	glActiveTexture(GL_TEXTURE0);
	if (SceneSystem && SceneSystem->Main)
		glBindTexture(GL_TEXTURE_2D, SceneSystem->Main->Buffer->Textures[0]);
	else
		glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, MainWindow->UI.UITextures[0]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, MainWindow->UI.UITextures[1]);

	PostProcess->SetInt("u_texture", 0);
	PostProcess->SetInt("u_ui", 1);
	PostProcess->SetInt("u_alpha", 2);

#if EDITOR
	if (EditorSubsystem::Active)
	{
		editor::Viewport* View = editor::Viewport::Current;

		PostProcess->SetVec2("u_pos", View->ViewportBackground->GetScreenPosition() / 2 + 0.5);
		PostProcess->SetVec2("u_size", View->ViewportBackground->GetUsedSize() / 2);
	}
	else
#endif
	{
		PostProcess->SetVec2("u_pos", 0);
		PostProcess->SetVec2("u_size", 1);
	}

	glDrawArrays(GL_TRIANGLES, 0, 3);
	SDL_GL_SetSwapInterval(1);
	SDL_GL_SwapWindow(GetSysWindow(MainWindow)->SDLWindow);
}

void engine::subsystem::VideoSubsystem::OnResized()
{
	for (auto& Callback : OnResizedCallbacks)
	{
		Callback(MainWindow->GetSize());
	}
}
