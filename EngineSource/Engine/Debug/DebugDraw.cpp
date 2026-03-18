#include "DebugDraw.h"
#include <Engine/File/ModelData.h>
#include <Engine/Scene.h>

using namespace engine::graphics;

engine::debug::DebugDraw::DebugDraw()
{
}

engine::debug::DebugDraw::~DebugDraw()
{
}

void engine::debug::DebugDraw::Draw(GraphicsScene* With)
{
	for (DebugShape* i : this->Shapes)
	{
		i->Draw(With);
	}
}

void engine::debug::DebugDraw::AddShape(DebugShape* Shape)
{
	Shape->Owner = this;
	this->Shapes.push_back(Shape);
}

void engine::debug::DebugDraw::RemoveShape(DebugShape* Shape)
{
	for (auto it = Shapes.begin(); it != Shapes.end(); it++)
	{
		if (*it == Shape)
		{
			Shapes.erase(it);
			return;
		}
	}
}

engine::debug::DebugBox::DebugBox(Vector3 Position, Rotation3 Rotation, Vector3 Scale, Vector3 Color)
	: DebugBox(Transform(Position, Rotation, Scale), Color)
{
}

engine::debug::DebugBox::DebugBox(Transform WithTransform, Vector3 Color)
{
	this->CubeModel = GraphicsModel::UnitCube()->Drawable;
	CubeTransform = WithTransform;

	CubeMaterial = new Material();
	CubeMaterial->VertexShader = "res:shader/basic.vert";
	CubeMaterial->FragmentShader = "res:shader/internal/debugShape.frag";
	CubeMaterial->UpdateShader();
	CubeMaterial->VerifyUniforms();
	SetColor(Color);
}

engine::debug::DebugBox::~DebugBox()
{
	delete CubeMaterial;
}

void engine::debug::DebugBox::Draw(GraphicsScene* With)
{
	std::vector<Material*> Materials = { CubeMaterial };
	this->CubeModel->Draw(With, CubeTransform, With->UsedCamera, Materials, BoundingBox(), false);
}

void engine::debug::DebugBox::SetColor(Vector3 NewColor)
{
	if (NewColor != CubeColor)
	{
		this->CubeColor = NewColor;
		CubeMaterial->SetVec3("u_color", CubeColor);
	}
}

engine::debug::DebugShape::~DebugShape()
{
	if (Owner)
	{
		Owner->RemoveShape(this);
	}
}
