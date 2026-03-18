#include "GraphicsScene.h"
#include <Engine/Graphics/VideoSubsystem.h>
#include <Engine/Internal/OpenGL.h>
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
		int64(FromSize.X * ViewportSize.X) >> 1,
		int64(FromSize.Y * ViewportSize.Y) >> 1
	);
}
#endif

engine::graphics::GraphicsScene::GraphicsScene()
{
}

engine::graphics::GraphicsScene::~GraphicsScene()
{
	delete Buffer;
	delete SceneCamera;
}

void engine::graphics::GraphicsScene::OnResized(kui::Vec2ui NewSize)
{
	if (BufferSize != 0)
	{
		this->PostProcess.OnBufferResized(uint32(BufferSize.X), uint32(BufferSize.Y));
		Buffer->Resize(int64(BufferSize.X), int64(BufferSize.Y));
	}
	else
	{
#if EDITOR
		if (editor::EditorSubsystem::Active)
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

void engine::graphics::GraphicsScene::AddDrawnComponent(DrawableComponent* New)
{
	DrawnComponents.push_back(New);
	NewDrawables.push_back(New);
}

void engine::graphics::GraphicsScene::Init()
{
	VideoSubsystem* VideoSystem = VideoSubsystem::Current;

	if (VideoSystem)
	{
		kui::Vec2ui BufferSize = VideoSystem->MainWindow->GetSize();
#if EDITOR
		if (editor::EditorSubsystem::Active)
		{
			BufferSize = GetEditorSize(BufferSize);
		}
#endif
		Buffer = new Framebuffer(int64(BufferSize.X), int64(BufferSize.Y));

		SceneCamera = new Camera(1);
		SceneCamera->Position.Z = 2;
		SceneCamera->Rotation.Y = -90;
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

		PostProcess.Init(uint32(BufferSize.X), uint32(BufferSize.Y));
		Shadows.Init();
		Lights.UpdateBounds(this);
		BuildBoundingVolume();
	}
}

void engine::graphics::GraphicsScene::RemoveDrawnComponent(DrawableComponent* Removed)
{
	for (auto i = DrawnComponents.begin(); i < DrawnComponents.end(); i++)
	{
		if (*i == Removed)
		{
			DrawnComponents.erase(i);
			break;
		}
	}

	RemovedDrawables.insert(Removed);
}

void engine::graphics::GraphicsScene::Draw()
{
	if (!AlwaysRedraw && !RedrawNextFrame)
		return;

	RedrawNextFrame = false;
	
	UpdateEnvironment();

	ShadowDrawPass();

	MainDrawPass();

	this->SceneTexture = PostProcess.Draw(Buffer, this->UsedCamera);

	// Start doing work for the next frame asynchronously
	BuildBoundingVolume();
	Lights.UpdateBounds(this);
}

static void ShowDebugNodes(GraphicsScene* With, BvhNode<DrawableComponent*>* Node, size_t Depth)
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
	glEnable(GL_CULL_FACE);
	glEnable(GL_STENCIL_TEST);
	glViewport(0, 0, GLsizei(Buffer->Width), GLsizei(Buffer->Height));
	glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
	glClearColor(0, 0, 0, 1);
	glStencilMask(0xFF);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glStencilMask(0);
	glStencilFunc(GL_GEQUAL, 1, 0xFF);

	bool LastIsTransparent = false;
	bool LastDrawStencil = false;

	std::vector<DrawableComponent*> ToDraw = NewDrawables;
	{
		std::lock_guard hg{ this->HierarchyMutex };
		this->DrawableHierarchy->QueryFrustum(UsedCamera->Collider, ToDraw);
	}

	using Entry = std::pair<SortingInfo, DrawableComponent*>;
	std::vector<Entry> SortedComponents;

	SortedComponents.reserve(ToDraw.size());
	for (size_t i = 0; i < ToDraw.size(); i++)
	{
		if (RemovedDrawables.contains(ToDraw[i]))
		{
			continue;
		}

		SortedComponents.push_back({
			{ ToDraw[i]->GetWorldTransform().ApplyTo(0), ToDraw[i]->DrawBoundingBox },
			ToDraw[i]
			});
	}

	std::sort(SortedComponents.begin(), SortedComponents.end(), [](const Entry& a, const Entry& b) -> bool {
		if (a.second->IsTransparent && !b.second->IsTransparent)
		{
			return false;
		}
		if (!a.second->IsTransparent && b.second->IsTransparent)
		{
			return true;
		}
		if (a.second->IsTransparent && b.second->IsTransparent)
		{
			// Draw transparent objects in reverse order
			return a.first.Position.X > b.first.Position.X;
		}

		return a.first.Position.X < b.first.Position.X;
	});

	for (auto& [_, i] : SortedComponents)
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
		if (i->IsTransparent && !LastIsTransparent)
		{
			glEnable(GL_BLEND);
			LastIsTransparent = true;
		}
		else if (!i->IsTransparent && LastDrawStencil)
		{
			glDisable(GL_BLEND);
			LastIsTransparent = false;
		}
		i->Draw(UsedCamera, this);
	}

	glEnable(GL_BLEND);
	Debug.Draw(this);
	glDisable(GL_BLEND);
	Buffer->Unbind();
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_CULL_FACE);
}

void engine::graphics::GraphicsScene::BuildBoundingVolume()
{
	if (RebuildingHierarchy)
	{
		return;
	}

	std::lock_guard g{ this->HierarchyMutex };
	RebuildingHierarchy = true;
	std::list<std::pair<DrawableComponent*, BoundingBox>> Components;

	for (auto& i : this->DrawnComponents)
	{
		Components.push_back({ i, i->DrawBoundingBox });
	}
	RemovedDrawables.clear();
	NewDrawables.clear();

	ThreadPool::Main()->AddJob([this, Components = std::move(Components)] {
		auto NewHierarchy = std::make_shared<BvhNode<DrawableComponent*>>(Components);

		std::lock_guard g{ this->HierarchyMutex };
		DrawableHierarchy = NewHierarchy;
		RebuildingHierarchy = false;
	});
}
