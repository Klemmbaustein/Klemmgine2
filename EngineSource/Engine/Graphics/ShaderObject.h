#pragma once
#include <Core/Types.h>
#include <Core/Vector.h>
#include <Core/Transform.h>
#include <unordered_map>

namespace engine::graphics
{
	class ShaderObject
	{
	public:
		static bool CheckCompileErrors(uint32 ShaderID, string Type);
		ShaderObject(string VertexFile, string FragmentFile, string GeometryFile = "");
		~ShaderObject();


		void ReCompile(string VertexFile, string FragmentFile);
		void Compile(string VertexFile, string FragmentFile, string GeometryFile = "");

		void Bind();

		uint32 ShaderID = 0;
		uint32 ModelUniform = 0;

		bool Valid = false;
		bool Unlit = false;

		uint32 GetUniformLocation(const string& Name) const
		{
			std::hash<std::string_view> h;
			return GetUniformLocation(h(Name), Name.c_str());
		}
		uint32 GetUniformLocation(const char* Name) const
		{
			std::hash<std::string_view> h;
			return GetUniformLocation(h(Name), Name);
		}

		uint32 GetUniformLocation(size_t NameHash, const char* Name) const;

		void SetInt(uint32 UniformLocation, int32 Value);
		void SetFloat(uint32 UniformLocation, float Value);
		void SetVec3(uint32 UniformLocation, Vector3 Value);
		void SetVec2(uint32 UniformLocation, Vector2 Value);
		void SetTransform(uint32 UniformLocation, const Transform& Value);
	private:

		mutable std::unordered_map<size_t, uint32> Uniforms;

		string VertexFile, FragmentFile, GeometryFile;
		void Clear();
	};
}