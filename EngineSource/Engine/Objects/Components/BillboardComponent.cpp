#include "BillboardComponent.h"
#include <Engine/Scene.h>

engine::BillboardComponent::BillboardComponent()
{
	this->IsTransparent = true;
	this->IsOpaque = false;
}

void engine::BillboardComponent::OnAttached()
{
}

void engine::BillboardComponent::OnDetached()
{
	if (this->Model)
	{
		GetRootObject()->GetScene()->Graphics.RemoveDrawnComponent(this);
	}
}

bool engine::BillboardComponent::UpdateTransform(bool IsDirty)
{
	bool Result = ObjectComponent::UpdateTransform(IsDirty);
	if (Model && (DrawBoundingBox.Extents == 0 || Result))
	{
		DrawBoundingBox = BoundingBox(0, 1).Translate(WorldTransform);
	}
	return Result;
}

void engine::BillboardComponent::LoadImage(AssetRef Image)
{
	if (GetRootObject()->GetScene())
	{
		for (auto& i : this->Materials)
		{
			delete i;
		}

		this->Model = GraphicsModel::Billboard();

		this->Materials = { graphics::Material::MakeBillboard(Image.FilePath) };

		GetRootObject()->GetScene()->Graphics.AddDrawnComponent(this);
	}
}

void engine::BillboardComponent::SetColor(Vector3 NewColor)
{
	this->Materials[0]->SetVec3("u_color", NewColor);
}

void engine::BillboardComponent::Draw(graphics::Renderer* Render, graphics::Camera* From, graphics::GraphicsScene* In)
{
}

void engine::BillboardComponent::DrawTransparent(graphics::Renderer* Render, graphics::Camera* From, graphics::GraphicsScene* In)
{
	Vector3 Position, Scale;
	Rotation3 Rotation;
	WorldTransform.Decompose(Position, Rotation, Scale);

	Transform AtTransform = Transform(Position, 0, Scale * 0.25f).Combine(Transform(From->View).Rotation().Inverse());

	this->Model->Drawable->Draw(Render, &GetRootObject()->GetScene()->Graphics, AtTransform,
		From, this->Materials, this->DrawBoundingBox, this->DrawStencil, true);
}
