#include "MeshComponent.h"
#include <Engine/Scene.h>
#ifdef EDITOR
#include "CollisionComponent.h"
#include <Engine/Engine.h>
#endif

using namespace engine::graphics;

void engine::MeshComponent::Update()
{
	if (DrawnModel && DrawnModel->Data)
		DrawnModel->Data->CastShadow = CastShadow;
}

void engine::MeshComponent::Draw(Renderer* Render, Camera* From, GraphicsScene* In)
{
	if (this->IsVisible && DrawnModel && DrawnModel->Drawable)
	{
		IsTransparent = false;
		for (auto& i : this->Materials)
		{
			if (i->IsTransparent)
			{
				IsTransparent = true;
			}
		}

		DrawnModel->Drawable->Draw(Render, DrawAsOpaqueStencil ? nullptr : In, WorldTransform,
			From, Materials, DrawBoundingBox, DrawStencil, false);
	}
}

void engine::MeshComponent::DrawTransparent(Renderer* Render, Camera* From, ::GraphicsScene* In)
{
	if (this->IsVisible && DrawnModel && DrawnModel->Drawable)
	{
		DrawnModel->Drawable->Draw(Render, DrawAsOpaqueStencil ? nullptr : In, WorldTransform,
			From, Materials, DrawBoundingBox, DrawStencil, true);
	}
}

void engine::MeshComponent::LoadMaterial(size_t MaterialIndex, AssetRef MaterialFile)
{
	auto Previous = this->Materials[MaterialIndex];
	if (Previous)
	{
		delete Previous;
	}

	this->Materials[MaterialIndex] = new Material(MaterialFile);
}

void engine::MeshComponent::SimpleDraw(graphics::Renderer* Render, ShaderObject* With)
{
	if (DrawnModel && (RootObject || ParentObject))
	{
		DrawnModel->Drawable->SimpleDraw(Render, WorldTransform, With, Materials);
	}
}

void engine::MeshComponent::Load(AssetRef From, bool LoadMaterials)
{
	Load(GraphicsModel::GetModel(From, LoadMaterials));
}

void engine::MeshComponent::Load(GraphicsModel* From)
{
	if (IsRegistered)
	{
		ClearModel(false);
	}

	DrawnModel = From;

	if (DrawnModel)
	{
		auto UpdateMaterials = [this] {
			for (auto& i : this->Materials)
			{
				delete i;
			}
			Materials.clear();
			for (auto& m : DrawnModel->Data->Meshes)
			{
				if (m.Material.empty())
				{
					Materials.push_back(graphics::Material::MakeDefault());
					continue;
				}
				AssetRef Ref = AssetRef::FromName(m.Material, "kmt");

				if (Ref.Exists())
					Materials.push_back(new graphics::Material(Ref));
				else
				{
					Ref = AssetRef::FromName(m.Material, "kbm");
					Materials.push_back(new graphics::Material(Ref));
				}
			}
		};

		DrawnModel->OnMaterialsChanged.Add(this, UpdateMaterials);

		UpdateMaterials();
	}

	if (DrawnModel && DrawnModel->Drawable)
	{
		if (!IsRegistered && (RootObject || ParentObject))
			InitializeModel();
		IsRegistered = true;
		this->CastShadow = DrawnModel->Data->CastShadow;
	}
}

void engine::MeshComponent::OnAttached()
{
	if (IsRegistered && (RootObject || ParentObject))
		InitializeModel();
}

engine::MeshComponent::~MeshComponent()
{
	ClearModel(true);

	if (Debug)
	{
		delete Debug;
	}
}

bool engine::MeshComponent::UpdateTransform(bool IsDirty)
{
	bool Result = ObjectComponent::UpdateTransform(IsDirty);
	if (DrawnModel && (DrawBoundingBox.Extents == 0 || Result))
	{
		DrawBoundingBox = DrawnModel->Data->Bounds.Translate(WorldTransform);
	}
	return Result;
}

void engine::MeshComponent::ClearModel(bool RemoveDrawnComponent)
{
	if (DrawnModel)
	{
		DrawnModel->OnMaterialsChanged.Remove(this);
		GraphicsModel::UnloadModel(DrawnModel);
	}
	DrawnModel = nullptr;

	for (graphics::Material* Mat : Materials)
	{
		delete Mat;
	}

	Materials.clear();

	if (RemoveDrawnComponent && (RootObject || ParentObject))
	{
		GetRootObject()->GetScene()->Graphics.RemoveDrawnComponent(this);
		IsRegistered = false;
	}
}

void engine::MeshComponent::InitializeModel()
{
	auto RootScene = GetRootObject()->GetScene();
	RootScene->Graphics.AddDrawnComponent(this);
#ifdef EDITOR
	if (!Engine::IsPlaying && RootScene->Physics.Active)
	{
		auto ViewCollider = new CollisionComponent();
		this->Attach(ViewCollider);

		GraphicsModel::ReferenceModel(DrawnModel);
		ViewCollider->Load(this->DrawnModel, true);
	}
#endif
}
