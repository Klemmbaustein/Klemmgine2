#include "Scene.h"
#include <GL/glew.h>
#include "Engine/Subsystem/VideoSubsystem.h"
#include <Engine/Subsystem/SceneSubsystem.h>
#include "Engine/Engine.h"
#include "Engine/Input.h"
#include "Stats.h"
#include <Engine/File/TextSerializer.h>
#include <Engine/File/BinarySerializer.h>
#include <Engine/Subsystem/EditorSubsystem.h>
#include <Engine/Editor/UI/Viewport.h>

std::atomic<int32> engine::Scene::AsyncLoads = 0;

#if EDITOR

static kui::Vec2i GetEditorSize(kui::Vec2ui FromSize)
{
	using namespace engine;

	kui::Vec2f ViewportSize = editor::Viewport::Current->ViewportBackground->GetUsedSize();

	return kui::Vec2i(
		int64(FromSize.X * ViewportSize.X) >> 1,
		int64(FromSize.Y * ViewportSize.Y) >> 1
	);
}
#endif


engine::Scene::Scene(bool DoLoadAsync)
{
	if (!DoLoadAsync)
	{
		Init();
	}
}

engine::Scene::Scene(string FilePath)
{
	LoadInternal(FilePath, false);
	Init();
}

engine::Scene::Scene(const char* FilePath)
	: Scene(string(FilePath))
{
}

void engine::Scene::Draw()
{	
	Cam->Update();

	Buffer->Bind();

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, Buffer->Width, Buffer->Height);
	for (SceneObject* i : Objects)
	{
		if (i->HasVisuals)
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

	if (!SceneSubsystem::Current)
		return nullptr;

	return SceneSubsystem::Current->Main;
}

void engine::Scene::OnResized(kui::Vec2ui NewSize)
{
	using namespace subsystem;

#if EDITOR
	if (EditorSubsystem::Active)
	{
		auto Size = GetEditorSize(NewSize);
		Buffer->Resize(Size.X, Size.Y);
		Cam->Aspect = float(Size.X) / float(Size.Y);
		return;
	}
#endif
	Buffer->Resize(NewSize.X, NewSize.Y);
	Cam->Aspect = float(Buffer->Width) / float(Buffer->Height);
}

void engine::Scene::LoadAsync(string SceneFile)
{
	AsyncLoads++;
	LoadInternal(SceneFile, true);
}

void engine::Scene::LoadAsyncFinish()
{
	AsyncLoads--;
	Init();
	for (SceneObject* obj : Objects)
	{
		obj->Begin();
	}
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

void engine::Scene::LoadInternal(string File, bool Async)
{
	SerializedValue SceneData = TextSerializer::FromFile(File);

	Name = File;

	if (SceneData.GetObject().empty())
		return;

	for (auto& i : SceneData.At("objects").GetArray())
	{
		ObjectTypeID ID = i.At("typeId").GetInt();

		const Reflection::ObjectInfo& Type = Reflection::ObjectTypes[ID];
		SceneObject* Object = Type.CreateInstance();
		Object->DeSerialize(&i);
		Object->InitObj(this, !Async, Type.TypeID);

		this->Objects.push_back(Object);
	}
}

void engine::Scene::Init()
{
	using namespace graphics;
	using namespace subsystem;

	VideoSubsystem* VideoSystem = Engine::GetSubsystem<VideoSubsystem>();

	kui::Vec2ui BufferSize = VideoSystem->MainWindow->GetSize();
#if EDITOR
	if (EditorSubsystem::Active)
	{
		BufferSize = GetEditorSize(BufferSize);
	}
#endif
	Buffer = new Framebuffer(int64(BufferSize.X), int64(BufferSize.Y));

	Cam = new Camera(1);
	Cam->Position.Z = 2;
	Cam->Rotation.Y = 3.14f / -2.0f;

	Cam->Aspect = float(Buffer->Width) / float(Buffer->Height);

	VideoSystem->OnResizedCallbacks.push_back(
		[this](kui::Vec2ui NewSize)
		{
			OnResized(NewSize);
		});
	SceneSubsystem::Current->LoadedScenes.push_back(this);
}

engine::SerializedValue engine::Scene::GetSceneInfo()
{
	return SerializedValue();
}
