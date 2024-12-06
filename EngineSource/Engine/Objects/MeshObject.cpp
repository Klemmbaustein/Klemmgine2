#include "MeshObject.h"
#include <kui/Resource.h>

void engine::MeshObject::LoadMesh(string Name)
{
	if (DrawnModel)
	{
		GraphicsModel::UnloadModel(DrawnModel);
	}

	ModelName = AssetRef::FromPath(Name);

	DrawnModel = GraphicsModel::GetModel(ModelName.Value);

	Shader = new graphics::ShaderObject(
		{ kui::resource::GetStringFile("res:shader/basic.vert") },
		{ kui::resource::GetStringFile("res:shader/basic.frag") }
	);
}

void engine::MeshObject::Draw(graphics::Camera* From)
{
	if (DrawnModel)
		DrawnModel->Drawable->Draw(Position, From, Shader);
}

void engine::MeshObject::Begin()
{
	ModelName.OnChanged = [this]() {
		LoadMesh(ModelName.Value.FilePath);
		};
	HasVisuals = true;
	ModelName.OnChanged();
}
