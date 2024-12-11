#pragma once
#include <Engine/Types.h>
#include <vector>
#include <Engine/Vector.h>

namespace engine::graphics
{
	class ShaderObject
	{
		void CheckCompileErrors(uint32 ShaderID, string Type);
	public:
		ShaderObject(std::vector<string> VertexFiles, std::vector<string> FragmentFiles);

		void Bind();

		uint32 ShaderID = 0;

		uint32 GetUniformLocation(string Name) const;

		void SetInt(uint32 UniformLocation, uint32 Value);
		void SetFloat(uint32 UniformLocation, float Value);
		void SetVec3(uint32 UniformLocation, Vector3 Value);
	};
}