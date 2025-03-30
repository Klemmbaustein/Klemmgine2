#include "MeshObject.h"

void engine::MeshObject::LoadMesh(AssetRef File)
{
	ModelName.Value = File;
	Mesh->Load(File);
	if (Mesh->DrawnModel && Mesh->DrawnModel->Data->HasCollision)
	{
		Collider->Load(File);
	}
}

void engine::MeshObject::Begin()
{
	Mesh = new MeshComponent();
	Attach(Mesh);

	Collider = new CollisionComponent();
	Attach(Collider);

	ModelName.OnChanged = [this]() {
		LoadMesh(ModelName.Value);
	};
	ModelName.OnChanged();
}

void engine::MeshObject::OnDestroyed()
{
}
