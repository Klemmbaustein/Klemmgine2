#include "MeshObject.h"
#include <kui/Resource.h>
#include <Engine/Log.h>

void engine::MeshObject::LoadMesh(AssetRef File)
{
	if (DrawnModel)
	{
		Destroy();
	}

	ModelName = File;

	DrawnModel = GraphicsModel::GetModel(ModelName.Value);

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

void engine::MeshObject::Draw(graphics::Camera* From)
{
	if (DrawnModel)
	{
		DrawnModel->Drawable->Draw(Position, From, Materials);
	}
}

void engine::MeshObject::Begin()
{
	ModelName.OnChanged = [this]() {
		LoadMesh(ModelName.Value);
	};
	HasVisuals = true;
	ModelName.OnChanged();
}

void engine::MeshObject::Destroy()
{
	if (DrawnModel)
		GraphicsModel::UnloadModel(DrawnModel);

	for (graphics::Material* Mat : Materials)
	{
		delete Mat;
	}

	Materials.clear();
}
