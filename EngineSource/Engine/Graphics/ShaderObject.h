#pragma once
#include <Engine/Types.h>
#include <vector>

namespace engine::graphics
{
	class ShaderObject
	{
		void CheckCompileErrors(uint32 ShaderID, string Type);
	public:
		ShaderObject(std::vector<string> VertexFiles, std::vector<string> FragmentFiles);

		void Bind();

		uint32 ShaderID = 0;
	};
}