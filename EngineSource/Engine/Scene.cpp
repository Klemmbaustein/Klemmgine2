#include "Scene.h"
#include <GL/glew.h>
#include "Engine/Subsystem/VideoSubsystem.h"
#include <Engine/Subsystem/SceneSubsystem.h>
#include "Engine/Engine.h"
#include "Engine/Input.h"
#include "Stats.h"
#include <Engine/File/TextSerializer.h>
#include <Engine/File/BinarySerializer.h>

engine::Scene::Scene()
{
	using namespace graphics;
	using namespace subsystem;

	VideoSubsystem* VideoSystem = Engine::GetSubsystem<VideoSubsystem>();

	kui::Vec2ui BufferSize = VideoSystem->MainWindow->GetSize();
	Buffer = new Framebuffer(uint64(BufferSize.X), uint64(BufferSize.Y));

	Cam = new Camera(1);
	Cam->Position.Z = 2;
	Cam->Rotation.Y = 3.14f / -2.0f;

	Cam->Aspect = float(Buffer->Width) / float(Buffer->Height);

	VideoSystem->OnResizedCallbacks.push_back(
		[this](kui::Vec2ui NewSize)
		{
			Buffer->Resize(NewSize.X, NewSize.Y);
			Cam->Aspect = float(Buffer->Width) / float(Buffer->Height);
		});
}

void engine::Scene::Draw()
{	
	Cam->Update();

	Buffer->Bind();

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (graphics::IDrawable* i : Drawables)
	{
		i->Draw(Cam);
	}
}

void engine::Scene::Update()
{
	if (input::IsKeyDown(input::Key::w))
	{
		Cam->Position += Vector3::Forward(Cam->Rotation) * Vector3(stats::DeltaTime * 5);
	}
	if (input::IsKeyDown(input::Key::s))
	{
		Cam->Position -= Vector3::Forward(Cam->Rotation) * Vector3(stats::DeltaTime * 5);
	}
	if (input::IsKeyDown(input::Key::d))
	{
		Cam->Position += Vector3::Right(Cam->Rotation) * Vector3(stats::DeltaTime * 5);
	}
	if (input::IsKeyDown(input::Key::a))
	{
		Cam->Position -= Vector3::Right(Cam->Rotation) * Vector3(stats::DeltaTime * 5);
	}

	Cam->Rotation = Cam->Rotation + Vector3(input::MouseMovement.Y, input::MouseMovement.X, 0);
}

engine::Scene* engine::Scene::GetMain()
{
	using namespace subsystem;

	SceneSubsystem* SceneSystem = Engine::GetSubsystem<SceneSubsystem>();

	if (!SceneSystem)
		return nullptr;

	return SceneSystem->Main;
}

void engine::Scene::Save(string FileName)
{
	std::vector<SerializedValue> SerializedObjects;

	for (SceneObject* Object : this->Objects)
	{
		SerializedObjects.push_back(Object->Serialize());
	}

	SerializedValue Serialized = SerializedValue(
		{
			SerializedData("scene", GetSceneInfo()),
			SerializedData("objects", SerializedObjects),
		});

	TextSerializer::ToFile(Serialized.GetObject(), FileName);
}

engine::SerializedValue engine::Scene::GetSceneInfo()
{
	return SerializedValue();
}
