#include "Scene.h"
#include <GL/glew.h>
#include "Engine/Subsystem/VideoSubsystem.h"
#include <Engine/Subsystem/SceneSubsystem.h>
#include "Engine/Engine.h"
#include <Engine/File/ModelData.h>
#include <Engine/File/TextSerializer.h>
#include <Engine/File/BinarySerializer.h>
#include <Engine/Subsystem/EditorSubsystem.h>
#include <Engine/Editor/UI/Panels/Viewport.h>

std::atomic<int32> engine::Scene::AsyncLoads = 0;

#if EDITOR

static kui::Vec2i GetEditorSize(kui::Vec2ui FromSize)
{
	using namespace engine;

	if (!editor::Viewport::Current)
		return FromSize;

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

engine::Scene::~Scene()
{
	using namespace subsystem;

	delete Buffer;
	delete SceneCamera;

	SceneSubsystem* Sys = SceneSubsystem::Current;

	if (Sys->Main == this)
	{
		Sys->Main = nullptr;
	}

	for (auto i = Sys->LoadedScenes.begin(); i < Sys->LoadedScenes.end(); i++)
	{
		if (*i == this)
		{
			Sys->LoadedScenes.erase(i);
			break;
		}
	}

	for (auto& i : ReferencedAssets)
	{
		GraphicsModel::UnloadModel(i);
	}

	for (auto& i : Objects)
	{
		i->OnDestroyed();
		i->ClearComponents();
		delete i;
	}

	VideoSubsystem* VideoSystem = Engine::GetSubsystem<VideoSubsystem>();

	VideoSystem->OnResizedCallbacks.erase(this);
}

engine::Scene::Scene(const char* FilePath)
	: Scene(string(FilePath))
{
}

void engine::Scene::Draw()
{
	SceneCamera->Update();

	Buffer->Bind();

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glViewport(0, 0, GLsizei(Buffer->Width), GLsizei(Buffer->Height));
	for (SceneObject* i : Objects)
	{
		i->Draw(UsedCamera);
	}
	glDisable(GL_CULL_FACE);
}
void engine::Scene::Update()
{
	for (SceneObject* Obj : Objects)
	{
		Obj->UpdateObject();
	}

	for (SceneObject* Destr : DestroyedObjects)
	{
		Destr->OnDestroyed();
		Destr->ClearComponents();

		for (auto i = Objects.begin(); i < Objects.end(); i++)
		{
			if (*i == Destr)
			{
				Objects.erase(i);
				break;
			}
		}
		delete Destr;
	}
	DestroyedObjects.clear();
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

	if (BufferSize != 0)
	{
		Buffer->Resize(int64(BufferSize.X), int64(BufferSize.Y));
	}
	else
	{
#if EDITOR
		if (EditorSubsystem::Active)
		{
			auto Size = GetEditorSize(NewSize);
			Buffer->Resize(Size.X, Size.Y);
			SceneCamera->Aspect = float(Size.X) / float(Size.Y);
			return;
		}
#endif
		Buffer->Resize(NewSize.X, NewSize.Y);
	}
	SceneCamera->Aspect = float(Buffer->Width) / float(Buffer->Height);
}

void engine::Scene::CreateObjectFromID(uint32 ID, Vector3 Position, Rotation3 Rotation, Vector3 Scale)
{
	const Reflection::ObjectInfo& Type = Reflection::ObjectTypes[ID];
	SceneObject* Object = Type.CreateInstance();
	Object->OriginScene = this;
	Object->Position = Position;
	Object->Rotation = Rotation;
	Object->Scale = Scale;
	Object->InitObj(this, true, Type.TypeID);
	this->Objects.push_back(Object);
}

void engine::Scene::ReloadObjects(SerializedValue* FromState)
{
	if (FromState)
	{
		for (SceneObject* i : Objects)
		{
			i->OnDestroyed();
			i->ClearComponents();
			delete i;
		}
		Objects.clear();

		std::vector<AssetRef> OldReferenced = ReferencedAssets;
		ReferencedAssets.clear();

		DeSerializeInternal(FromState, false);

		for (auto& i : OldReferenced)
		{
			GraphicsModel::UnloadModel(i);
		}
	}
	else
	{
		for (SceneObject* i : Objects)
		{
			i->OnDestroyed();
			i->ClearComponents();
		}

		for (SceneObject* i : Objects)
		{
			i->Begin();
		}
	}
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

	Update();
}

engine::SerializedValue engine::Scene::Serialize()
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

	
	return Serialized;
}

void engine::Scene::DeSerialize(SerializedValue* From)
{
	DeSerializeInternal(From, false);
}

void engine::Scene::Save(string FileName)
{
	using namespace subsystem;

	SceneSubsystem::Current->Print(str::Format("Saving scene: %s", FileName.c_str()), Subsystem::LogType::Info);

	TextSerializer::ToFile(Serialize().GetObject(), FileName);
}

bool engine::Scene::ObjectDestroyed(SceneObject* Target) const
{
	return DestroyedObjects.contains(Target);
}

void engine::Scene::PreLoadAsset(AssetRef Target)
{
	if (Target.Extension == "kmdl")
	{
		GraphicsModel::RegisterModel(Target);
	}
	ReferencedAssets.push_back(Target);
}

void engine::Scene::DeSerializeInternal(SerializedValue* From, bool Async)
{
	if (From->GetType() != SerializedData::DataType::Object || From->GetObject().empty())
		return;

	for (auto& i : From->At("objects").GetArray())
	{
		try
		{
			ObjectTypeID ID = i.At("typeId").GetInt();

			const Reflection::ObjectInfo& Type = Reflection::ObjectTypes[ID];
			SceneObject* Object = Type.CreateInstance();
			Object->OriginScene = this;
			Object->DeSerialize(&i);
			Object->InitObj(this, !Async, Type.TypeID);

			this->Objects.push_back(Object);
		}
		catch (SerializeException& SerializeErr)
		{
			subsystem::SceneSubsystem::Current->Print(
				str::Format("Failed to DeSerialize object in scene: '%s'", SerializeErr.what()),
				subsystem::Subsystem::LogType::Warning
			);
		}
	}
}

void engine::Scene::LoadInternal(string File, bool Async)
{
	SerializedValue SceneData;
	Name = File;
	try
	{
		SceneData = TextSerializer::FromFile(File);
	}
	catch (SerializeReadException& ReadErr)
	{
		subsystem::SceneSubsystem::Current->Print(
			str::Format("Failed read scene file %s: %s", File.c_str(), ReadErr.what()),
			subsystem::Subsystem::LogType::Error
		);
		return;
	}

	DeSerializeInternal(&SceneData, Async);

	if (!Async)
	{
		Update();
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

	SceneCamera = new Camera(1);
	SceneCamera->Position.Z = 2;
	SceneCamera->Rotation.Y = 3.14f / -2.0f;

	SceneCamera->Aspect = float(Buffer->Width) / float(Buffer->Height);
	UsedCamera = SceneCamera;
	SceneSubsystem::Current->LoadedScenes.push_back(this);

	VideoSystem->OnResizedCallbacks.insert({ this,
		[this](kui::Vec2ui NewSize)
		{
			if (Resizable)
				OnResized(NewSize);
		} });
	Physics.Init();
}

engine::SerializedValue engine::Scene::GetSceneInfo()
{
	return SerializedValue();
}
