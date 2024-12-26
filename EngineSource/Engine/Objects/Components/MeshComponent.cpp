#include "MeshComponent.h"

void engine::MeshComponent::Update()
{
}

void engine::MeshComponent::Draw(graphics::Camera* From)
{
	if (DrawnModel)
	{
		DrawnModel->Drawable->Draw(WorldTransform, From, Materials);
	}
}

void engine::MeshComponent::Load(AssetRef From)
{
	if (DrawnModel)
	{
		ClearModel();
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
}

engine::MeshComponent::~MeshComponent()
{
	ClearModel();
}

void engine::MeshComponent::ClearModel()
{
	if (DrawnModel)
		GraphicsModel::UnloadModel(DrawnModel);
	DrawnModel = nullptr;

	for (graphics::Material* Mat : Materials)
	{
		delete Mat;
	}

	Materials.clear();
}
