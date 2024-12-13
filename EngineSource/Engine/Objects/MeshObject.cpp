#include "MeshObject.h"
#include <kui/Resource.h>

void engine::MeshObject::LoadMesh(AssetRef File)
{
	if (DrawnModel)
	{
		GraphicsModel::UnloadModel(DrawnModel);
	}

	ModelName = File;

	DrawnModel = GraphicsModel::GetModel(ModelName.Value);

	Mat = new graphics::Material("Assets/material.kmt");
}

void engine::MeshObject::Draw(graphics::Camera* From)
{
	if (DrawnModel && Mat && Mat->Shader)
	{
		Mat->Apply();
		DrawnModel->Drawable->Draw(Position, From, Mat->Shader);
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
	if (Mat)
		delete Mat;
}
