#pragma once
#include "Vertex.h"
#include <vector>

namespace engine::graphics
{
	struct VertexBuffer
	{
		uint32 VAO = 0u, VBO = 0u, EBO = 0u, IndicesSize = 0u;
		VertexBuffer(const std::vector<Vertex>& Vertices, const std::vector<uint32>& Indices);
		~VertexBuffer();
		VertexBuffer(const VertexBuffer& Other) = delete;

		void BindBuffer();
		void Draw();
	};
}