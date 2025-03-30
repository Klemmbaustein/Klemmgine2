#include "MeshComponent.h"
#include <Engine/Scene.h>

void engine::MeshComponent::Update()
{
}

void engine::MeshComponent::Draw(graphics::Camera* From)
{
	auto Root = GetRootObject();

	DrawBoundingBox.Extents = Root->Scale;

	if (DrawnModel)
	{
		DrawnModel->Drawable->Draw(Root->GetScene(), WorldTransform, From, Materials);
	}
}

void engine::MeshComponent::SimpleDraw(graphics::ShaderObject* With)
{
	if (DrawnModel)
	{
		DrawnModel->Drawable->SimpleDraw(WorldTransform, With, Materials);
	}
}

void engine::MeshComponent::Load(AssetRef From)
{
	bool AlreadyRegistered = DrawnModel;
	if (AlreadyRegistered)
	{
		ClearModel(false);
	}

	DrawnModel = GraphicsModel::GetModel(From);

	if (DrawnModel && DrawnModel->Drawable)
	{
		for (auto& m : DrawnModel->Data->Meshes)
		{
			if (m.Material.empty())
			{
				Materials.push_back(graphics::Material::MakeDefault());
				continue;
			}
			AssetRef Ref = AssetRef::FromName(m.Material, "kmt");

			if (Ref.Exists())
				Materials.push_back(new graphics::Material(Ref));
			else
			{
				Ref = AssetRef::FromName(m.Material, "kbm");
				Materials.push_back(new graphics::Material(Ref));
			}
		}

		if (!AlreadyRegistered)
			GetRootObject()->GetScene()->AddDrawnComponent(this);
		CastShadow = DrawnModel->Data->CastShadow;
	}
	DrawBoundingBox.Position = 0;
}

engine::MeshComponent::~MeshComponent()
{
	ClearModel(true);
}

void engine::MeshComponent::ClearModel(bool RemoveDrawnComponent)
{
	if (DrawnModel)
		GraphicsModel::UnloadModel(DrawnModel);
	DrawnModel = nullptr;

	for (graphics::Material* Mat : Materials)
	{
		delete Mat;
	}

	Materials.clear();

	if (RemoveDrawnComponent)
	{
		GetRootObject()->GetScene()->RemoveDrawnComponent(this);
	}
}
