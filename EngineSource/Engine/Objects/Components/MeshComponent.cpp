#include "MeshComponent.h"
#include <Engine/Scene.h>

void engine::MeshComponent::Update()
{
}

void engine::MeshComponent::Draw(graphics::Camera* From)
{
	DrawBoundingBox.Extents = GetRootObject()->Scale;

	if (DrawnModel)
	{
		DrawnModel->Drawable->Draw(WorldTransform, From, Materials);
	}
}

void engine::MeshComponent::Load(AssetRef From)
{
	if (DrawnModel)
	{
		ClearModel(false);
	}
	else
	{
		GetRootObject()->GetScene()->AddDrawnComponent(this);
	}

	DrawnModel = GraphicsModel::GetModel(From);

	if (DrawnModel)
	{
		for (auto& m : DrawnModel->Data->Meshes)
		{
			if (m.Material.empty())
			{
				Materials.push_back(graphics::Material::MakeDefault());
				continue;
			}
			Materials.push_back(new graphics::Material(m.Material));
		}
	}
	DrawBoundingBox.Position = 0;
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

	if (RemoveDrawnComponent)
	{
		GetRootObject()->GetScene()->RemoveDrawnComponent(this);
	}
}
