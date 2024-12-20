#pragma once
#include <Engine/Types.h>
#include <vector>
#include <Engine/Vector.h>

namespace engine::graphics
{
	class ShaderObject
	{
	public:
		static bool CheckCompileErrors(uint32 ShaderID, string Type);
		ShaderObject(string VertexFile, string FragmentFile);
		~ShaderObject();


		void ReCompile();
		void Compile(string VertexFile, string FragmentFile);

		void Bind();

		uint32 ShaderID = 0;
		bool Valid = false;

		uint32 GetUniformLocation(string Name) const;

		void SetInt(uint32 UniformLocation, uint32 Value);
		void SetFloat(uint32 UniformLocation, float Value);
		void SetVec3(uint32 UniformLocation, Vector3 Value);
	private:
		string VertexFile, FragmentFile;
		void Clear();
	};
}