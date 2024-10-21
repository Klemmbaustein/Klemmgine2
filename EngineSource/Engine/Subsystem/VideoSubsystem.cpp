#include "VideoSubsystem.h"
#include "SceneSubsystem.h"
#include <kui/App.h>
#include <SDL3/SDL.h>
#include <GL/glew.h>
#include <Engine/Internal/SystemWM_SDL3.h>
#include <Engine/Engine.h>
#include <Engine/Stats.h>
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
		((engine::subsystem::VideoSubsystem*)userParam)->Print(std::string(message), engine::subsystem::Subsystem::LogType::Error);
	}
}

static systemWM::SysWindow* GetSysWindow(kui::Window* From)
{
	return static_cast<systemWM::SysWindow*>(From->GetSysWindow());
}

engine::subsystem::VideoSubsystem::VideoSubsystem()
	: Subsystem("Video")
{
	app::error::SetErrorCallback([this](string Message, bool Fatal)
		{
			Print("kui error: " + Message, Fatal ? LogType::Critical : LogType::Error);
		});

	Print("Initializing SDL", LogType::Info);
	SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO);

	Print("Creating main window", LogType::Info);
	MainWindow = new Window("Klemmgine 2", Window::WindowFlag::Resizable);

	MainWindow->OnResizedCallback = [this](Window*)
		{
			Print("Main window resized", LogType::Note);
			OnResized();
		};

	MainWindow->Markup.SetDefaultFont(new Font("res:DefaultFont.ttf"));

	MainWindow->UI.DrawToWindow = false;
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);
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
	stats::DeltaTime = MainWindow->GetDeltaTime();
	stats::Time += stats::DeltaTime;
}

void engine::subsystem::VideoSubsystem::RenderUpdate()
{
	SceneSubsystem* SceneSystem = Engine::GetSubsystem<SceneSubsystem>();

	if (SceneSystem)
	{
		for (Scene* scn : SceneSystem->LoadedScenes)
		{
			scn->Draw();
		}
	}

	auto PostProcess = MainWindow->Shaders.LoadShader("shader/postProcess.vert", "shader/drawToWindow.frag", "engineToWindow");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	PostProcess->Bind();
	glActiveTexture(GL_TEXTURE0);
	if (SceneSystem && SceneSystem->Main)
		glBindTexture(GL_TEXTURE_2D, SceneSystem->Main->Buffer->Texture);
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
