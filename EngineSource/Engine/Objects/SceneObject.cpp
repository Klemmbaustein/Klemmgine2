#include "SceneObject.h"
#include <iostream>
using namespace engine;

SerializedValue engine::SceneObject::Serialize()
{
	std::cout << Name << std::endl;
	return SerializedValue({
		SerializedData("position", Position),
		SerializedData("name", Name),
		SerializedData("typeId", int32(TypeID))
		});
}

void engine::SceneObject::DeSerialize(SerializedValue* From)
{
	Position = From->At("position").GetVector3();
	Name = From->At("name").GetString();
	TypeID = ObjectTypeID(From->At("typeId").GetInt());
}

Scene* engine::SceneObject::GetScene()
{
	return OriginScene;
}

void engine::SceneObject::Draw(graphics::Camera* Cam)
{
}

void engine::SceneObject::Begin()
{
}
