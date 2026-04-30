#include "Scene.h"
#include "Engine.h"
#include "Subsystem/SceneSubsystem.h"
#include "Graphics/VideoSubsystem.h"
#include <Core/File/BinarySerializer.h>
#include <Core/File/TextSerializer.h>
#include <Engine/File/ModelData.h>
#include <Engine/File/Resource.h>
#include <Engine/Graphics/Texture.h>
#include <Engine/Plugins/PluginLoader.h>
#include <filesystem>
#include <Engine/UI/UICanvas.h>
#include <Engine/Sound/SoundSubsystem.h>

#if EDITOR
#include <Editor/UI/Panels/Viewport.h>
#endif

using namespace engine::graphics;
using namespace engine;

std::atomic<int32> engine::Scene::AsyncLoads = 0;

#if EDITOR

static kui::Vec2i GetEditorSize(kui::Vec2ui FromSize)
{
	if (!editor::Viewport::Current)
		return FromSize;

	kui::Vec2f ViewportSize = editor::Viewport::Current->ViewportBackground->GetUsedSize().GetScreen();

	return kui::Vec2i(
		int64(FromSize.X * ViewportSize.X) >> 1,
		int64(FromSize.Y * ViewportSize.Y) >> 1
	);
}
#endif


engine::Scene::Scene(bool DoLoadAsync)
{
	auto System = Engine::GetSubsystem<sound::SoundSubsystem>();

	if (System)
	{
		Sound = new sound::SoundContext(System->MainDevice);
	}

	if (!DoLoadAsync)
	{
		Init();
	}
}

engine::Scene::Scene(string FilePath)
{
	auto System = Engine::GetSubsystem<sound::SoundSubsystem>();

	if (System)
	{
		Sound = new sound::SoundContext(System->MainDevice);
	}

	LoadInternal(FilePath, false);
	Init();
}

engine::Scene::~Scene()
{
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
		UnloadAsset(i);
	}

	for (auto& i : Objects)
	{
		i->OnDestroyed();
		i->OnDestroyedEvent.Invoke();
		i->ClearComponents();
		delete i;
	}

	VideoSubsystem* VideoSystem = Engine::GetSubsystem<VideoSubsystem>();
	UICanvas::ClearAll();

	delete Sound;

	VideoSystem->OnResizedCallbacks.erase(this);
}

engine::Scene::Scene(const char* FilePath)
	: Scene(string(FilePath))
{
}

void engine::Scene::Update()
{
	if (Physics.Active)
		Physics.Update();

	if (Sound)
		Sound->Update(this->Graphics.UsedCamera);

	for (size_t i = 0; i < Objects.size(); i++)
	{
		Objects[i]->UpdateObject();
	}

	for (SceneObject* Destroyed : DestroyedObjects)
	{
		Destroyed->OnDestroyed();
		Destroyed->OnDestroyedEvent.Invoke();
		Destroyed->ClearComponents();

		for (auto i = Objects.begin(); i < Objects.end(); i++)
		{
			if (*i == Destroyed)
			{
				Objects.erase(i);
				break;
			}
		}
		delete Destroyed;
	}
	DestroyedObjects.clear();
}

engine::Scene* engine::Scene::GetMain()
{
	if (!SceneSubsystem::Current)
		return nullptr;

	return SceneSubsystem::Current->Main;
}

engine::SceneObject* engine::Scene::CreateObjectFromID(int32 ID, Vector3 Position, Rotation3 Rotation, Vector3 Scale)
{
	const Reflection::ObjectInfo& Type = Reflection::ObjectTypes[ID];
	SceneObject* Object = Type.CreateInstance();
	Object->OriginScene = this;
	Object->Position = Position;
	Object->Rotation = Rotation;
	Object->Scale = Scale;
	Object->InitObj(this, true, Type.TypeID);
	this->Objects.push_back(Object);
	return Object;
}

void engine::Scene::ReloadObjects(SerializedValue* FromState)
{
	UICanvas::ClearAll();

	if (FromState)
	{
		for (SceneObject* i : Objects)
		{
			i->OnDestroyed();
			i->OnDestroyedEvent.Invoke();
			i->ClearComponents();
			delete i;
		}
		Objects.clear();

		std::vector<SceneAsset> OldReferenced = ReferencedAssets;
		ReferencedAssets.clear();

		DeSerializeInternal(FromState, false);

		for (auto& i : OldReferenced)
		{
			UnloadAsset(i);
		}
	}
	else
	{
		for (size_t i = 0; i < Objects.size(); i++)
		{
			Objects[i]->OnDestroyed();
			Objects[i]->OnDestroyedEvent.Invoke();
			Objects[i]->BeginCalled = false;
			Objects[i]->ClearComponents();
		}

		// Begin() might create new objects in some cases, which would invalidate the iterator
		// if we looped over the array directly. Objects spawned like this will already call Begin()
		// themselves so it doesn't matter we're not going over them here.
		std::vector ObjCopy = Objects;
		for (SceneObject* i : ObjCopy)
		{
			i->Begin();
			i->BeginCalled = true;
		}
	}
	plugin::OnNewSceneLoaded(this);
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
		obj->CheckTransform();
		obj->Begin();
		obj->BeginCalled = true;
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

engine::string engine::Scene::SaveToString(string FileName)
{
	using namespace subsystem;

	std::stringstream Stream;
	SceneSubsystem::Current->Print(str::Format("Saving scene: %s", FileName.c_str()), Subsystem::LogType::Info);

	TextSerializer::ToStream(Serialize().GetObject(), Stream);
	return Stream.str();
}

bool engine::Scene::ObjectDestroyed(SceneObject* Target) const
{
	return DestroyedObjects.contains(Target);
}

void engine::Scene::PreLoadAsset(AssetRef Target)
{
	void* TargetPtr = nullptr;
	if (Target.Extension == "kmdl")
	{
		auto Model = GraphicsModel::RegisterModel(Target);
		if (Model)
		{
			Model->Data->PreLoadMaterials(this);
			if (Physics.Active)
			{
				Physics.PreLoadMesh(Model);
			}
			TargetPtr = Model;
		}
	}
	if (Target.Extension == "png" && graphics::TextureLoader::Instance)
	{
		TargetPtr = const_cast<Texture*>(graphics::TextureLoader::Instance->PreLoadBuffer(Target,
			graphics::TextureOptions{}));
	}
	if (Target.Extension == "wav" && this->Sound)
	{
		TargetPtr = Sound->LoadSoundEffect(Target.FilePath);
	}
	ReferencedAssets.push_back({ Target, TargetPtr });
}

void engine::Scene::UnloadAsset(const SceneAsset& Target)
{
	using namespace engine::graphics;

	if (Target.FileReference.Extension == "kmdl")
	{
		GraphicsModel::UnloadModel(Target.FileReference);
	}
	if (Target.FileReference.Extension == "png")
	{
		TextureLoader::Instance->FreeTexture(reinterpret_cast<const Texture*>(Target.LoadedData));
	}
	if (Target.FileReference.Extension == "wav")
	{
		Sound->FreeSoundEffect(reinterpret_cast<sound::SoundBuffer*>(Target.LoadedData));
	}
}

void engine::Scene::DeSerializeInternal(SerializedValue* From, bool Async)
{
	if (From->GetType() != SerializedData::DataType::Object || From->GetObject().empty())
		return;

	try
	{
		auto& SceneInfo = From->At("scene");

		if (SceneInfo.Contains("sunColor"))
		{
			this->Graphics.SceneEnvironment.SunColor = SceneInfo.At("sunColor").GetVector3();
		}
		if (SceneInfo.Contains("sunRotation"))
		{
			this->Graphics.SceneEnvironment.SunRotation = SceneInfo.At("sunRotation").GetVector3();
		}

		if (SceneInfo.Contains("skyColor"))
		{
			this->Graphics.SceneEnvironment.SkyColor = SceneInfo.At("skyColor").GetVector3();
		}

		if (SceneInfo.Contains("groundColor"))
		{
			this->Graphics.SceneEnvironment.GroundColor = SceneInfo.At("groundColor").GetVector3();
		}

		if (SceneInfo.Contains("sunIntensity"))
		{
			this->Graphics.SceneEnvironment.SunIntensity = SceneInfo.At("sunIntensity").GetFloat();
		}
		if (SceneInfo.Contains("ambientIntensity"))
		{
			this->Graphics.SceneEnvironment.AmbientIntensity = SceneInfo.At("ambientIntensity").GetFloat();
		}
		if (SceneInfo.Contains("fogColor"))
		{
			this->Graphics.SceneEnvironment.FogColor = SceneInfo.At("fogColor").GetVector3();
		}
		if (SceneInfo.Contains("fogRange"))
		{
			this->Graphics.SceneEnvironment.FogRange = SceneInfo.At("fogRange").GetFloat();
		}
		if (SceneInfo.Contains("fogStart"))
		{
			this->Graphics.SceneEnvironment.FogStart = SceneInfo.At("fogStart").GetFloat();
		}

		if (SceneInfo.Contains("manager"))
		{
			const Reflection::ObjectInfo& Type = Reflection::ObjectTypes[SceneInfo.At("manager").GetInt()];
			SceneObject* Object = Type.CreateInstance();
			Object->OriginScene = this;
			Object->InitObj(this, true, Type.TypeID);

		}
	}
	catch (SerializeException& SerializeError)
	{
		SceneSubsystem::Current->Print(
			str::Format("Failed to read scene metadata: '%s'", SerializeError.what()),
			subsystem::Subsystem::LogType::Warning
		);
	}

	for (auto& i : From->At("objects").GetArray())
	{
		try
		{
			ObjectTypeID ID = i.At("typeId").GetInt();

			const Reflection::ObjectInfo& Type = Reflection::ObjectTypes.at(ID);
			SceneObject* Object = Type.CreateInstance();
			Object->OriginScene = this;
			Object->DeSerialize(&i);
			Object->InitObj(this, !Async, Type.TypeID);

			this->Objects.push_back(Object);
		}
		catch (SerializeException& SerializeError)
		{
			SceneSubsystem::Current->Print(
				str::Format("Failed to DeSerialize object in scene: '%s'", SerializeError.what()),
				subsystem::Subsystem::LogType::Warning
			);
		}
		catch (std::out_of_range& RangeError)
		{
			SceneSubsystem::Current->Print(
				str::Format("Out of range error while trying to DeSerialize object in scene: '%s'. Object likely had an invalid ID.",
					RangeError.what()),
				subsystem::Subsystem::LogType::Warning
			);
		}
	}
}

void engine::Scene::LoadInternal(string File, bool Async)
{
	Physics.Init();

	SerializedValue SceneData;
	Name = File;

	File = File.substr(0, File.find_last_of('.'));

	try
	{
		AssetRef SceneAsset = AssetRef::Convert(File + ".kts");
		if (SceneAsset.Exists())
		{
			Name = SceneAsset.FilePath;
			std::stringstream stream;
			stream << resource::GetTextFile(SceneAsset.FilePath);

			if (stream.str().size() != 0)
			{
				SceneData = TextSerializer::FromStream(stream);
			}
		}
		SceneAsset = AssetRef::Convert(File + ".kbs");

		if (SceneAsset.Exists())
		{
			Name = SceneAsset.FilePath;
			IBinaryStream* SceneFile = resource::GetBinaryFile(Name);
			SceneData = BinarySerializer::FromStream(SceneFile, "kbs");
			delete SceneFile;
		}
	}
	catch (SerializeReadException& ReadErr)
	{
		SceneSubsystem::Current->Print(
			str::Format("Failed read scene file %s: %s", File.c_str(), ReadErr.what()),
			subsystem::Subsystem::LogType::Error
		);
		return;
	}
	resource::LoadSceneFiles(Name);

	DeSerializeInternal(&SceneData, Async);

	if (!Async)
	{
		Update();
	}
}

void engine::Scene::Init()
{
	Graphics.Init();
	SceneSubsystem::Current->LoadedScenes.push_back(this);
	plugin::OnNewSceneLoaded(this);
}

engine::SerializedValue engine::Scene::GetSceneInfo()
{
	return std::vector{
		SerializedData("sunColor", this->Graphics.SceneEnvironment.SunColor),
		SerializedData("sunRotation", this->Graphics.SceneEnvironment.SunRotation.EulerVector()),
		SerializedData("skyColor", this->Graphics.SceneEnvironment.SkyColor),
		SerializedData("groundColor", this->Graphics.SceneEnvironment.GroundColor),
		SerializedData("sunIntensity", this->Graphics.SceneEnvironment.SunIntensity),
		SerializedData("ambientIntensity", this->Graphics.SceneEnvironment.AmbientIntensity),
		SerializedData("fogColor", this->Graphics.SceneEnvironment.FogColor),
		SerializedData("fogRange", this->Graphics.SceneEnvironment.FogRange),
		SerializedData("fogStart", this->Graphics.SceneEnvironment.FogStart),
	};
}
