#include "MeshObject.h"
#include <Engine/File/ModelData.h>
#include <kui/Resource.h>
#include <iostream>

void engine::MeshObject::LoadMesh(string Name)
{
	ModelName = AssetRef(Name);

	GraphicsModel* LoadedModel = GraphicsModel::GetModel(ModelName.Value.FilePath);

	Shader = new graphics::ShaderObject(
		{ kui::resource::GetStringFile("res:shader/basic.vert") },
		{ kui::resource::GetStringFile("res:shader/basic.frag") }
	);

	if (LoadedModel)
		DrawnModel = LoadedModel->Drawable;
}

void engine::MeshObject::Draw(graphics::Camera* From)
{
	if (DrawnModel)
		DrawnModel->Draw(Position, From, Shader);
}

void engine::MeshObject::Begin()
{
	HasVisuals = true;
	LoadMesh(ModelName.Value.FilePath);
}
