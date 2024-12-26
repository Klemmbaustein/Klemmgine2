#pragma once
#include "ObjectComponent.h"

namespace engine
{
	class CameraComponent : public ObjectComponent
	{
	public:
		CameraComponent();

		void Use();
	};
}