#pragma once
#include "SceneObject.h"

namespace engine
{
	class LightObject : public SceneObject
	{
	public:

		ENGINE_OBJECT(LightObject, "Engine");

		void Begin() override;
		void OnDestroyed() override;

		ObjProperty<float> Range = ObjProperty<float>("Range", 5.0f, this);
		ObjProperty<float> Intensity = ObjProperty<float>("Intensity", 1.0f, this);
		ObjProperty<Vector3> Color = ObjProperty<Vector3>("Color", 1.0f, this);

	private:

	};
}