#pragma once
#include <Core/Types.h>
#include <Core/Vector.h>
#include <Core/Transform.h>
#include <Engine/Graphics/Backend/Renderer.h>
#include <unordered_map>

namespace engine::graphics
{
	class ShaderObject
	{
	public:
		ShaderObject(string VertexFile, string FragmentFile, string GeometryFile = "");
		~ShaderObject();

		void ReCompile(string VertexFile, string FragmentFile);
		void Compile(string VertexFile, string FragmentFile, string GeometryFile = "");

		void Bind();

		ShaderProgram* Program = 0;
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

		uint32 GetUniformBlockLocation(const string& Name);

		uint32 GetUniformLocation(size_t NameHash, const char* Name) const;

		void SetInt(uint32 UniformLocation, int32 Value)
		{
			Program->SetInt(UniformLocation, Value);
		}
		void SetFloat(uint32 UniformLocation, float Value)
		{
			Program->SetFloat(UniformLocation, Value);
		}
		void SetVec3(uint32 UniformLocation, Vector3 Value)
		{
			Program->SetVec3(UniformLocation, Value);
		}
		void SetVec2(uint32 UniformLocation, Vector2 Value)
		{
			Program->SetVec2(UniformLocation, Value);
		}
		void SetTransform(uint32 UniformLocation, const Transform& Value)
		{
			Program->SetMatrix(UniformLocation, Value.Matrix);
		}
		void SetMatrix(uint32 UniformLocation, const glm::mat4& Value)
		{
			Program->SetMatrix(UniformLocation, Value);
		}

	private:
		mutable std::unordered_map<size_t, uint32> Uniforms;
		mutable std::unordered_map<string, uint32> UniformBlocks;

		string VertexFile, FragmentFile, GeometryFile;
		void Clear();
	};
}