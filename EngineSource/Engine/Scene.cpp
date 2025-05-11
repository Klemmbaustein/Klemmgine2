#include "Scene.h"
#include "Core/ThreadPool.h"
#include "Engine.h"
#include "Internal/OpenGL.h"
#include "Subsystem/SceneSubsystem.h"
#include "Subsystem/VideoSubsystem.h"
#include <algorithm>
#include <Core/File/BinarySerializer.h>
#include <Core/File/TextSerializer.h>
#include <Engine/File/ModelData.h>
#include <Engine/File/Resource.h>
#include <Engine/Graphics/Texture.h>
#include <Engine/Plugins/PluginLoader.h>
#include <filesystem>
#include <Engine/UI/UICanvas.h>

#if EDITOR
#include <Engine/Subsystem/EditorSubsystem.h>
#include <Editor/UI/Panels/Viewport.h>
#endif

std::atomic<int32> engine::Scene::AsyncLoads = 0;

#if EDITOR

static kui::Vec2i GetEditorSize(kui::Vec2ui FromSize)
{
	using namespace engine;

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
	using namespace engine::subsystem;

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
		UnloadAsset(i);
	}

	for (auto& i : Objects)
	{
		i->OnDestroyed();
		i->ClearComponents();
		delete i;
	}

	VideoSubsystem* VideoSystem = Engine::GetSubsystem<VideoSubsystem>();
	UICanvas::ClearAll();

	VideoSystem->OnResizedCallbacks.erase(this);
}

engine::Scene::Scene(const char* FilePath)
	: Scene(string(FilePath))
{
}

void engine::Scene::Draw()
{
	if (!AlwaysRedraw && !Redraw)
		return;

	Redraw = false;

	UsedCamera->Update();

	Shadows.Update(UsedCamera);

	{
		std::unique_lock g{ DrawSortMutex };

		Shadows.Draw(DrawnComponents);
	}

	Buffer->Bind();
	glEnable(GL_CULL_FACE);
	glEnable(GL_STENCIL_TEST);
	glViewport(0, 0, GLsizei(Buffer->Width), GLsizei(Buffer->Height));
	glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
	glClearColor(0, 0, 0, 1);
	glStencilMask(0xFF);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glStencilMask(0);
	glStencilFunc(GL_GEQUAL, 1, 0xFF);

	{
		bool LastDrawStencil = false;
		std::unique_lock g{ DrawSortMutex };

		for (DrawableComponent* i : DrawnComponents)
		{
			if (i->DrawStencil && !LastDrawStencil)
			{
				glStencilMask(1);
				LastDrawStencil = true;
			}
			else if (!i->DrawStencil && LastDrawStencil)
			{
				glStencilMask(0);
				LastDrawStencil = false;
			}
			i->Draw(UsedCamera);
		}
	}

	this->SceneTexture = PostProcess.Draw(Buffer, this->UsedCamera);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_CULL_FACE);
	StartSorting();
}

void engine::Scene::Update()
{
	if (Physics.Active)
		Physics.Update();

	for (SceneObject* Obj : Objects)
	{
		Obj->UpdateObject();
	}

	for (SceneObject* Destroyed : DestroyedObjects)
	{
		Destroyed->OnDestroyed();
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
	using namespace engine::subsystem;

	if (!SceneSubsystem::Current)
		return nullptr;

	return SceneSubsystem::Current->Main;
}

void engine::Scene::OnResized(kui::Vec2ui NewSize)
{
	using namespace engine::subsystem;

	if (BufferSize != 0)
	{
		this->PostProcess.OnBufferResized(uint32(BufferSize.X), uint32(BufferSize.Y));
		Buffer->Resize(int64(BufferSize.X), int64(BufferSize.Y));
	}
	else
	{
#if EDITOR
		if (EditorSubsystem::Active)
		{
			auto Size = GetEditorSize(NewSize);
			this->PostProcess.OnBufferResized(uint32(Size.X), uint32(Size.Y));
			Buffer->Resize(Size.X, Size.Y);
			SceneCamera->Aspect = float(Size.X) / float(Size.Y);
			return;
		}
#endif
		this->PostProcess.OnBufferResized(uint32(NewSize.X), uint32(NewSize.Y));
		Buffer->Resize(int64(NewSize.X), int64(NewSize.Y));
	}

	SceneCamera->Aspect = float(Buffer->Width) / float(Buffer->Height);
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
			i->ClearComponents();
			delete i;
		}
		Objects.clear();

		std::vector<AssetRef> OldReferenced = ReferencedAssets;
		ReferencedAssets.clear();

		DeSerializeInternal(FromState, false);

		for (auto& i : OldReferenced)
		{
			UnloadAsset(i);
		}
	}
	else
	{
		for (SceneObject* i : Objects)
		{
			i->OnDestroyed();
			i->BeginCalled = false;
			i->ClearComponents();
		}

		for (SceneObject* i : Objects)
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

bool engine::Scene::ObjectDestroyed(SceneObject* Target) const
{
	return DestroyedObjects.contains(Target);
}

void engine::Scene::AddDrawnComponent(DrawableComponent* New)
{
	std::unique_lock g{ DrawSortMutex };
	DrawnComponents.push_back(New);
	SortedComponents.push_back({ { New->GetWorldTransform().ApplyTo(0), New->DrawBoundingBox }, New });
}

void engine::Scene::RemoveDrawnComponent(DrawableComponent* Removed)
{
	std::unique_lock g{ DrawSortMutex };

	for (auto i = DrawnComponents.begin(); i < DrawnComponents.end(); i++)
	{
		if (*i == Removed)
		{
			DrawnComponents.erase(i);
			break;
		}
	}

	for (auto i = SortedComponents.begin(); i < SortedComponents.end(); i++)
	{
		if (i->second == Removed)
		{
			SortedComponents.erase(i);
			break;
		}
	}
}

void engine::Scene::PreLoadAsset(AssetRef Target)
{
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
		}
	}
	if (Target.Extension == "png" && graphics::TextureLoader::Instance)
	{
		graphics::TextureLoader::Instance->PreLoadBuffer(Target,
			graphics::TextureOptions{});
	}
	ReferencedAssets.push_back(Target);
}

void engine::Scene::UnloadAsset(AssetRef Target)
{
	if (Target.Extension == "kmdl")
	{
		GraphicsModel::UnloadModel(Target);
	}
	if (Target.Extension == "png")
	{
	}
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

			const Reflection::ObjectInfo& Type = Reflection::ObjectTypes.at(ID);
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
		catch (std::out_of_range& RangeError)
		{
			subsystem::SceneSubsystem::Current->Print(
				str::Format("Out of range error while trying to DeSerialize object in scene: '%s'. Object likely had an invalid ID.", RangeError.what()),
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
	try
	{
		if (std::filesystem::exists(File))
		{
			SceneData = TextSerializer::FromFile(Name);
		}
		else if (std::filesystem::exists(File + ".kts"))
		{
			Name = File + ".kts";
			SceneData = TextSerializer::FromFile(Name);
		}
		else if (resource::FileExists(File + ".kbs"))
		{
			if (resource::LoadedAssets.contains(File + ".kbs"))
			{
				Name = resource::LoadedAssets[File + ".kbs"];
			}
			else
			{
				Name = File + ".kbs";
			}
			IBinaryStream* SceneFile = resource::GetBinaryFile(Name);
			SceneData = BinarySerializer::FromStream(SceneFile, "kbs");
			delete SceneFile;
		}
	}
	catch (SerializeReadException& ReadErr)
	{
		subsystem::SceneSubsystem::Current->Print(
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
	using namespace engine::graphics;
	using namespace engine::subsystem;

	VideoSubsystem* VideoSystem = Engine::GetSubsystem<VideoSubsystem>();

	if (VideoSystem)
	{
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

		VideoSystem->OnResizedCallbacks.insert({ this,
			[this](kui::Vec2ui NewSize)
			{
				if (Resizable)
					OnResized(NewSize);
			} });

		PostProcess.Init(uint32(BufferSize.X), uint32(BufferSize.Y));
		Shadows.Init();
	}

	SceneSubsystem::Current->LoadedScenes.push_back(this);
	plugin::OnNewSceneLoaded(this);
}

engine::SerializedValue engine::Scene::GetSceneInfo()
{
	return std::vector{
		SerializedData("sunColor", Vector3(1, 0.5f, 0)),
	};
}

void engine::Scene::StartSorting()
{
	if (DrawnComponents.size() < 2)
		return;

	if (IsSorting)
		return;

	SortedComponents.resize(DrawnComponents.size());
	for (size_t i = 0; i < DrawnComponents.size(); i++)
	{
		SortedComponents[i] = {
			{ DrawnComponents[i]->GetWorldTransform().ApplyTo(0), DrawnComponents[i]->DrawBoundingBox },
			DrawnComponents[i]
		};
	}

	Vector3 CameraPosition = SceneCamera->Position;
	IsSorting = true;

	ThreadPool::Main()->AddJob([this, CameraPosition]()
		{
			using Entry = std::pair<SortingInfo, ObjectComponent*>;

			std::unique_lock g{ DrawSortMutex };
			for (auto& i : SortedComponents)
			{
				Vector3 BoundsPosition = i.first.Position + i.first.Bounds.Position;
				i.first.Position.X = Vector3::Distance(BoundsPosition, CameraPosition) - i.first.Bounds.Extents.Length();
			}

			std::sort(SortedComponents.begin(), SortedComponents.end(), [](const Entry& a, const Entry& b) -> bool
				{
					return a.first.Position.X < b.first.Position.X;
				});
			IsSorting = false;

			for (size_t i = 0; i < DrawnComponents.size(); i++)
			{
				DrawnComponents[i] = SortedComponents[i].second;
			}
		});
}
