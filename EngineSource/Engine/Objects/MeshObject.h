#pragma once
#include "SceneObject.h"
#include <Engine/Graphics/Drawable/Model.h>

namespace engine
{
	class MeshObject : public SceneObject
	{
	public:
		string ModelName;
		graphics::Model* DrawnModel = nullptr;

		void LoadMesh(string Name);

		SerializedValue Serialize() override;
		void Begin() override;
	};
}