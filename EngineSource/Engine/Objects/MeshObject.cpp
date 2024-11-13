#include "MeshObject.h"
#include <Engine/Scene.h>

void engine::MeshObject::LoadMesh(string Name)
{
	ModelName = Name;

	auto Model = ModelData();
	Model.Meshes.push_back(ModelData::Mesh(std::vector<Vertex>{
			Vertex(Vector3(0.5f,  -0.5f, 0.0f)),
			Vertex(Vector3(0.5f, 0.5f, 0.0f)),
			Vertex(Vector3(-0.5f, -0.5f, 0.0f)),
			Vertex(Vector3(-0.5f,   0.5f, 0.0f)),
		},
		std::vector<uint32>{ 0, 1, 2, 1, 3, 2 }));

	Model.ToFile("Assets/test.kmdl");

	auto LoadedModel = ModelData("Assets/importTest.kmdl");

	DrawnModel = new graphics::Model(&LoadedModel);
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
