#include "MeshObject.h"
#include <kui/Resource.h>
#include <Engine/Log.h>

void engine::MeshObject::LoadMesh(AssetRef File)
{
	ModelName.Value = File;
	Mesh->Load(File);
}

void engine::MeshObject::Begin()
{
	Mesh = new MeshComponent();
	Attach(Mesh);

	ModelName.OnChanged = [this]() {
		LoadMesh(ModelName.Value);
	};
	ModelName.OnChanged();
}

void engine::MeshObject::OnDestroyed()
{
}
