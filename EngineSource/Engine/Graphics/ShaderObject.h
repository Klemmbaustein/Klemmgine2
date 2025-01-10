#pragma once
#include <Engine/Types.h>
#include <Engine/Vector.h>
#include <Engine/Transform.h>

namespace engine::graphics
{
	class ShaderObject
	{
	public:
		static bool CheckCompileErrors(uint32 ShaderID, string Type);
		ShaderObject(string VertexFile, string FragmentFile, string GeometryFile = "");
		~ShaderObject();


		void ReCompile();
		void Compile(string VertexFile, string FragmentFile, string GeometryFile = "");

		void Bind();

		uint32 ShaderID = 0;
		bool Valid = false;

		uint32 GetUniformLocation(string Name) const;

		void SetInt(uint32 UniformLocation, uint32 Value);
		void SetFloat(uint32 UniformLocation, float Value);
		void SetVec3(uint32 UniformLocation, Vector3 Value);
		void SetTransform(uint32 UniformLocation, const Transform& Value);
	private:
		string VertexFile, FragmentFile, GeometryFile;
		void Clear();
	};
}