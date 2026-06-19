#include "MeshObject.h"

void engine::MeshObject::LoadMesh(AssetRef File)
{
	ModelName.Value = File;
	Mesh->Load(File);
	if (Mesh->DrawnModel && Mesh->DrawnModel->Data->HasCollision)
	{
		Collider->Load(File);
	}

	if (!Mesh->DrawnModel)
	{
		return;
	}

	for (size_t i = 0; i < std::min(Mesh->Materials.size(), Materials.Values.size()); i++)
	{
		auto Item = Materials.GetValue(i);
		if (Item && Item->Value.IsValid())
		{
			Mesh->LoadMaterial(i, Item->Value);
		}
	}
	Materials.Values.resize(Mesh->Materials.size());

	size_t idx = 0;
	for (auto& i : Materials.Values)
	{
		if (!i)
		{
			i = std::make_shared<ObjProperty<AssetRef>>("", AssetRef::EmptyAsset("kmt"), nullptr);
		}
		i->OnChanged = Materials.OnChanged;
		i->Name = "Slot " + std::to_string(idx++);
		i->Hints = PropertyHint::AssetEmptyIsDefault;
	}
}

void engine::MeshObject::LoadData(GraphicsModel* Data)
{
	Mesh->Load(Data);
	if (Mesh->DrawnModel && Mesh->DrawnModel->Data->HasCollision)
	{
		Collider->Load(Data);
	}
}

void engine::MeshObject::Begin()
{
	Mesh = new MeshComponent();
	Attach(Mesh);

	Collider = new CollisionComponent();
	Attach(Collider);

	ModelName.OnChanged = [this]() {
		if (ModelName.Value.IsValid())
			LoadMesh(ModelName.Value);
	};
	ModelName.OnChanged();

	CastShadow.OnChanged = [this]() {
		Mesh->CastShadow = this->CastShadow.Value;
	};

	Materials.OnChanged = ModelName.OnChanged;
}

void engine::MeshObject::OnDestroyed()
{
}
