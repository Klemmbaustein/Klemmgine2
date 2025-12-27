#include "MeshComponent.h"
#include <Engine/Scene.h>

void engine::MeshComponent::Update()
{
	if (DrawnModel && DrawnModel->Data)
		DrawnModel->Data->CastShadow = CastShadow;
}

void engine::MeshComponent::Draw(graphics::Camera* From)
{
	auto Root = GetRootObject();

	if (this->IsVisible && DrawnModel && DrawnModel->Drawable)
	{
		DrawBoundingBox = DrawnModel->Data->Bounds;
		DrawnModel->Drawable->Draw(Root ? Root->GetScene() : nullptr, WorldTransform, From,
			Materials, this->DrawStencil);
	}
}

void engine::MeshComponent::SimpleDraw(graphics::ShaderObject* With)
{
	if (DrawnModel && (RootObject || ParentObject))
	{
		DrawnModel->Drawable->SimpleDraw(WorldTransform, With, Materials);
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

	if (DrawnModel && DrawnModel->Drawable)
	{
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

		if (!IsRegistered && (RootObject || ParentObject))
			GetRootObject()->GetScene()->AddDrawnComponent(this);
		IsRegistered = true;
		this->CastShadow = DrawnModel->Data->CastShadow;
	}
}

void engine::MeshComponent::OnAttached()
{
	if (IsRegistered && (RootObject || ParentObject))
		GetRootObject()->GetScene()->AddDrawnComponent(this);
}

engine::MeshComponent::~MeshComponent()
{
	ClearModel(true);
}

void engine::MeshComponent::ClearModel(bool RemoveDrawnComponent)
{
	if (DrawnModel)
		GraphicsModel::UnloadModel(DrawnModel);
	DrawnModel = nullptr;

	for (graphics::Material* Mat : Materials)
	{
		delete Mat;
	}

	Materials.clear();

	if (RemoveDrawnComponent && (RootObject || ParentObject))
	{
		GetRootObject()->GetScene()->RemoveDrawnComponent(this);
		IsRegistered = false;
	}
}
