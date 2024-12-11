#pragma once
#include "Engine/Vector.h"

namespace engine::graphics
{
	struct Vertex
	{
		Vector3 Position;
		Vector2 UV;
		Vector3 Normal;
	};
}