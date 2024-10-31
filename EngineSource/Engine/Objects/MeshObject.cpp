#include "MeshObject.h"
#include <Engine/Scene.h>

void engine::MeshObject::LoadMesh(string Name)
{
	ModelName = Name;
	DrawnModel = new graphics::Model();
	GetScene()->Drawables.push_back(DrawnModel);
}

engine::SerializedValue engine::MeshObject::Serialize()
{
	auto DefaultValues = SceneObject::Serialize();

	DefaultValues.Append(SerializedData("model", ModelName));
	return DefaultValues;
}

void engine::MeshObject::Begin()
{
	LoadMesh(Name);
}
