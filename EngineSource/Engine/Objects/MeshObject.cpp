#include "MeshObject.h"
#include <Engine/File/ModelData.h>
#include <kui/Resource.h>
#include <iostream>

void engine::MeshObject::LoadMesh(string Name)
{
	ModelName = Name;

	//Model.Meshes.push_back(ModelData::Mesh(std::vector<Vertex>{
	//		Vertex(Vector3(0.5f,  -0.5f, 0.0f)),
	//		Vertex(Vector3(0.5f, 0.5f, 0.0f)),
	//		Vertex(Vector3(-0.5f, -0.5f, 0.0f)),
	//		Vertex(Vector3(-0.5f,   0.5f, 0.0f)),
	//	},
	//	std::vector<uint32>{ 0, 1, 2, 1, 3, 2 }));

	GraphicsModel* LoadedModel = GraphicsModel::GetModel("Assets/importTest.kmdl");

	Shader = new ShaderObject(
		{ kui::resource::GetStringFile("res:shader/basic.vert") },
		{ kui::resource::GetStringFile("res:shader/basic.frag") }
	);

	DrawnModel = LoadedModel->Drawable;

}

void engine::MeshObject::Draw(Camera* From)
{
	DrawnModel->Draw(Position, From, Shader);
}

engine::SerializedValue engine::MeshObject::Serialize()
{
	auto DefaultValues = SceneObject::Serialize();

	DefaultValues.Append(SerializedData("model", ModelName));
	return DefaultValues;
}

void engine::MeshObject::Begin()
{
	HasVisuals = true;
	LoadMesh(Name);
}
