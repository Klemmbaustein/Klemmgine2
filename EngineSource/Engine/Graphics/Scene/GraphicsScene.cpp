#include "GraphicsScene.h"
#include <Engine/Graphics/VideoSubsystem.h>
#include <algorithm>
#include <Core/ThreadPool.h>

#if EDITOR
#include <Editor/EditorSubsystem.h>
#include <Editor/UI/Panels/Viewport.h>
#endif

using namespace engine;
using namespace engine::graphics;

#if EDITOR

static kui::Vec2i GetEditorSize(kui::Vec2ui FromSize)
{
	if (!editor::Viewport::Current)
		return FromSize;

	kui::Vec2f ViewportSize = editor::Viewport::Current->ViewportBackground->GetUsedSize().GetScreen();

	return kui::Vec2i(
		int64(std::round(FromSize.X * ViewportSize.X / 2.0f)),
		int64(std::round(FromSize.Y * ViewportSize.Y / 2.0f))
	);
}
#endif

engine::graphics::GraphicsScene::GraphicsScene()
{
}

engine::graphics::GraphicsScene::~GraphicsScene()
{
	*this->StopAsyncProcesses = true;
	delete Buffer;
	delete SceneCamera;
}

void engine::graphics::GraphicsScene::OnResized(kui::Vec2ui NewSize)
{
	if (BufferSize != 0)
	{
		Post.OnBufferResized(uint32(BufferSize.X), uint32(BufferSize.Y));
		Buffer->Resize(int64(BufferSize.X), int64(BufferSize.Y));
	}
	else
	{
#if EDITOR
		if (editor::EditorSubsystem::Active)
		{
			auto Size = GetEditorSize(NewSize);
			Post.OnBufferResized(uint32(Size.X), uint32(Size.Y));
			Buffer->Resize(Size.X, Size.Y);
			SceneCamera->Aspect = float(Size.X) / float(Size.Y);
			return;
		}
#endif
		Post.OnBufferResized(uint32(NewSize.X), uint32(NewSize.Y));
		Buffer->Resize(int64(NewSize.X), int64(NewSize.Y));
	}

	SceneCamera->Aspect = float(Buffer->Width) / float(Buffer->Height);
}

void engine::graphics::GraphicsScene::AddDrawnComponent(DrawableComponent* New)
{
	auto Drawable = DrawableData(New->UniqueId, New);

	DrawnComponents.push_back(Drawable);
	std::lock_guard g{ *this->HierarchyMutex };
	NewDrawables.push_back(Drawable);
}

void engine::graphics::GraphicsScene::Init()
{
	VideoSubsystem* VideoSystem = VideoSubsystem::Current;

	if (VideoSystem)
	{
		this->Render = VideoSystem->Renderer;
		kui::Vec2ui BufferSize = VideoSystem->MainWindow->GetSize();
#if EDITOR
		if (editor::EditorSubsystem::Active)
		{
			BufferSize = GetEditorSize(BufferSize);
		}
#endif
		Buffer = new Framebuffer(int64(BufferSize.X), int64(BufferSize.Y));

		SceneCamera = new Camera(70.0f);
		SceneCamera->Position.Y = 2.0f;

		SceneCamera->UsedEnvironment = &this->SceneEnvironment;

		SceneCamera->Aspect = float(Buffer->Width) / float(Buffer->Height);
		UsedCamera = SceneCamera;

		// Editor controls resizing of scenes otherwise
#if !EDITOR
		VideoSystem->OnResizedCallbacks.insert({ this,
			[this](kui::Vec2ui NewSize)
			{
				if (Resizable)
					OnResized(NewSize);
			} });
#endif

		Post.Init(uint32(BufferSize.X), uint32(BufferSize.Y));
		Shadows.Init(Render);
		Lights.UpdateBounds(this);
		BuildBoundingVolume();
	}
}

void engine::graphics::GraphicsScene::RemoveDrawnComponent(DrawableComponent* Removed)
{
	for (auto i = DrawnComponents.begin(); i < DrawnComponents.end(); i++)
	{
		if (i->Component == Removed)
		{
			DrawnComponents.erase(i);
			break;
		}
	}
	std::lock_guard g{ *this->HierarchyMutex };

	RemovedDrawableIds.insert(Removed->UniqueId);
}

void engine::graphics::GraphicsScene::Draw(Renderer* With)
{
	if (!AlwaysRedraw && !RedrawNextFrame)
		return;

	RedrawNextFrame = false;

	UpdateEnvironment();

	ShadowDrawPass();

	MainDrawPass();

	this->SceneTexture = Post.Draw(Buffer, this->UsedCamera);

	// Start doing work for the next frame asynchronously
	BuildBoundingVolume();
	Lights.UpdateBounds(this);
}

static void ShowDebugNodes(GraphicsScene* With, BvhNode<GraphicsScene::DrawableData>* Node, size_t Depth)
{
	if (Node->A)
	{
		ShowDebugNodes(With, Node->A, Depth + 1);
	}
	if (Node->B)
	{
		ShowDebugNodes(With, Node->B, Depth + 1);
	}

	static std::vector<Vector3> Colors = {
		Vector3(1, 0, 0),
		Vector3(0, 1, 0),
		Vector3(0, 0, 1),
		Vector3(1, 0, 1),
		Vector3(0, 1, 1),
		Vector3(1, 1, 0),
	};

	Node->Debug = new debug::DebugBox(Node->Bounds.Position, Rotation3(0),
		Node->Bounds.Extents - Depth * 0.01f, Colors[Depth % Colors.size()]);

	With->Debug.AddShape(Node->Debug);
}

void engine::graphics::GraphicsScene::UpdateEnvironment()
{
	UsedCamera->Update();

	if (UsedCamera->UseSceneEnvironment)
	{
		UsedCamera->UsedEnvironment = &this->SceneEnvironment;
	}
	else
	{
		UsedCamera->UsedEnvironment->NextFrame();
	}
	this->SceneEnvironment.NextFrame();
}

void engine::graphics::GraphicsScene::ShadowDrawPass()
{
	if (VideoSubsystem::Current->DrawShadows)
	{
		Shadows.Enabled = true;

		Shadows.Update(UsedCamera);
		Shadows.Draw(this);
	}
	else
	{
		Shadows.Enabled = false;
		Shadows.Update(UsedCamera);
	}
}

void engine::graphics::GraphicsScene::MainDrawPass()
{
	Buffer->Bind();

	Buffer->Buffer->Clear(true, true, 0xff);

	bool LastIsTransparent = false;

	std::vector<DrawableData> ToDraw;
	{
		std::lock_guard hg{ *this->HierarchyMutex };
		ToDraw = NewDrawables;
		if (this->DrawableHierarchy)
		{
			this->DrawableHierarchy->QueryFrustum(UsedCamera->Collider, ToDraw);
		}
	}

	using Entry = std::pair<SortingInfo, DrawableComponent*>;
	std::vector<Entry> SortedComponents;

	SortedComponents.reserve(ToDraw.size());
	{
		std::lock_guard g{ *this->HierarchyMutex };
		for (size_t i = 0; i < ToDraw.size(); i++)
		{
			if (RemovedDrawableIds.contains(ToDraw[i].Id))
			{
				continue;
			}

			SortedComponents.push_back({
				{ ToDraw[i].Component->GetWorldTransform().ApplyTo(0), ToDraw[i].Component->DrawBoundingBox },
				ToDraw[i].Component
				});
		}
	}

	std::sort(SortedComponents.begin(), SortedComponents.end(), [](const Entry& a, const Entry& b) -> bool {
		return a.first.Position.X < b.first.Position.X;
	});

	for (auto& [_, i] : SortedComponents)
	{
		if (i->IsOpaque)
		{
			i->Draw(Render, UsedCamera, this);
		}
	}

	for (auto i = SortedComponents.rbegin(); i != SortedComponents.rend(); i++)
	{
		if (i->second->IsTransparent)
		{
			i->second->DrawTransparent(Render, UsedCamera, this);
		}
	}
	Debug.Draw(this);
}

void engine::graphics::GraphicsScene::BuildBoundingVolume()
{
	if (RebuildingHierarchy)
	{
		return;
	}

	std::lock_guard g{ *this->HierarchyMutex };
	RebuildingHierarchy = true;
	std::list<std::pair<DrawableData, BoundingBox>> Components;

	for (auto& i : this->DrawnComponents)
	{
		if (i.Component->DrawBoundingBox.Extents != 0)
		{
			Components.push_back({ i, i.Component->DrawBoundingBox });
		}
	}

	// All new drawables that were new when the BVH building started
	std::vector<DrawableData> OldNewDrawables = NewDrawables;
	std::set<uint64> OldRemovedDrawables = RemovedDrawableIds;

	ThreadPool::Main()->AddJob(
		[this, Components = std::move(Components), OldNewDrawables = std::move(OldNewDrawables),
		OldRemovedDrawables = std::move(OldRemovedDrawables), Stop = StopAsyncProcesses, HierarchyMutex = HierarchyMutex] {

		auto NewHierarchy = std::make_shared<BvhNode<DrawableData>>(Components);

		std::lock_guard g{ *HierarchyMutex };

		if (*Stop)
		{
			return;
		}

		for (auto it = OldNewDrawables.begin(); it != OldNewDrawables.end(); it++)
		{
			for (auto it2 = NewDrawables.begin(); it2 != NewDrawables.end(); it2++)
			{
				if (*it2 == *it)
				{
					NewDrawables.erase(it2);
					break;
				}
			}
		}

		for (auto& i : OldRemovedDrawables)
		{
			RemovedDrawableIds.erase(i);
		}
		DrawableHierarchy = NewHierarchy;
		RebuildingHierarchy = false;
	});
}

SerializedValue engine::graphics::GraphicsScene::Serialize()
{
	return std::vector{
		SerializedData("sunColor", SceneEnvironment.SunColor),
		SerializedData("sunShadows", SceneEnvironment.Render.SunShadows),
		SerializedData("sunRotation", SceneEnvironment.SunRotation.EulerVector()),
		SerializedData("skyColor", SceneEnvironment.SkyColor),
		SerializedData("groundColor", SceneEnvironment.GroundColor),
		SerializedData("sunIntensity", SceneEnvironment.SunIntensity),
		SerializedData("ambientIntensity", SceneEnvironment.AmbientIntensity),
		SerializedData("fogColor", SceneEnvironment.FogColor),
		SerializedData("fogRange", SceneEnvironment.FogRange),
		SerializedData("fogStart", SceneEnvironment.FogStart),
		SerializedData("bloomEnabled", SceneEnvironment.Render.Bloom),
		SerializedData("bloomStrength", SceneEnvironment.Render.BloomStrength),
		SerializedData("bloomThreshold", SceneEnvironment.Render.BloomThreshold),
	};
}

void engine::graphics::GraphicsScene::DeSerialize(SerializedValue* From)
{
	if (!From->Contains("env"))
	{
		return;
	}

	auto& SceneInfo = From->At("env");
	if (SceneInfo.Contains("sunColor"))
	{
		SceneEnvironment.SunColor = SceneInfo.At("sunColor").GetVector3();
	}
	if (SceneInfo.Contains("sunRotation"))
	{
		SceneEnvironment.SunRotation = SceneInfo.At("sunRotation").GetVector3();
	}

	if (SceneInfo.Contains("skyColor"))
	{
		SceneEnvironment.SkyColor = SceneInfo.At("skyColor").GetVector3();
	}

	if (SceneInfo.Contains("groundColor"))
	{
		SceneEnvironment.GroundColor = SceneInfo.At("groundColor").GetVector3();
	}

	if (SceneInfo.Contains("sunIntensity"))
	{
		SceneEnvironment.SunIntensity = SceneInfo.At("sunIntensity").GetFloat();
	}
	if (SceneInfo.Contains("sunShadows"))
	{
		SceneEnvironment.Render.SunShadows = SceneInfo.At("sunShadows").GetBool();
	}
	if (SceneInfo.Contains("ambientIntensity"))
	{
		SceneEnvironment.AmbientIntensity = SceneInfo.At("ambientIntensity").GetFloat();
	}
	if (SceneInfo.Contains("fogColor"))
	{
		SceneEnvironment.FogColor = SceneInfo.At("fogColor").GetVector3();
	}
	if (SceneInfo.Contains("fogRange"))
	{
		SceneEnvironment.FogRange = SceneInfo.At("fogRange").GetFloat();
	}
	if (SceneInfo.Contains("fogStart"))
	{
		SceneEnvironment.FogStart = SceneInfo.At("fogStart").GetFloat();
	}
	if (SceneInfo.Contains("bloomEnabled"))
	{
		SceneEnvironment.Render.Bloom = SceneInfo.At("bloomEnabled").GetBool();
	}
	if (SceneInfo.Contains("bloomStrength"))
	{
		SceneEnvironment.Render.BloomStrength = SceneInfo.At("bloomStrength").GetFloat();
	}
	if (SceneInfo.Contains("bloomThreshold"))
	{
		SceneEnvironment.Render.BloomThreshold = SceneInfo.At("bloomThreshold").GetFloat();
	}

}
