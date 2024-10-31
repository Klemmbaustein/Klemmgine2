#pragma once
#include "Vertex.h"
#include <vector>

namespace engine::graphics
{
	struct MeshData
	{
		std::vector<Vertex> Vertices;
		std::vector<uint32> Indices;
	};
}