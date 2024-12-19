#include "ShaderObject.h"
#include <kui/Resource.h>
#include "ShaderLoader.h"
#include <Engine/Internal/OpenGL.h>
#include <Engine/Log.h>

void engine::graphics::ShaderObject::CheckCompileErrors(uint32 ShaderID, string Type)
{
	GLint success;
	GLchar infoLog[1024];
	if (Type != "PROGRAM")
	{
		glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(ShaderID, 1024, NULL, infoLog);
			Log::Error("Shader compilation error of type: "
				+ Type
				+ "\n"
				+ std::string(infoLog));
		}
	}
	else
	{
		glGetProgramiv(ShaderID, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(ShaderID, 1024, NULL, infoLog);
			Log::Error("Shader linking error of type: "
				+ Type
				+ "\n"
				+ std::string(infoLog));
		}
	}

}

engine::graphics::ShaderObject::ShaderObject(std::vector<string> VertexFiles, std::vector<string> FragmentFiles)
{
	std::vector<const char*> VertexCstringArray;

	VertexCstringArray.reserve(VertexFiles.size());
	for (auto& i : VertexFiles)
	{
		VertexCstringArray.push_back(i.c_str());
	}

	std::vector<const char*> FragmentCstringArray;

	std::vector<uint32> FragmentModules;

	VertexCstringArray.reserve(FragmentFiles.size());
	for (auto& i : FragmentFiles)
	{
		auto Res = ShaderLoader::Current->Modules.ParseShader(i);
		i = Res.ResultSource;
		for (auto& mod : Res.DependencyModules)
		{
			FragmentModules.push_back(mod.ModuleObject);
		}
		FragmentCstringArray.push_back(i.c_str());
	}

	unsigned int vertex, fragment;
	// vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, GLsizei(VertexCstringArray.size()), VertexCstringArray.data(), nullptr);
	glCompileShader(vertex);
	CheckCompileErrors(vertex, "VERTEX");
	// fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, GLsizei(FragmentCstringArray.size()), FragmentCstringArray.data(), nullptr);
	glCompileShader(fragment);
	CheckCompileErrors(fragment, "FRAGMENT");

	// shader Program
	ShaderID = glCreateProgram();
	glAttachShader(ShaderID, vertex);
	glAttachShader(ShaderID, fragment);

	for (uint32 mod : FragmentModules)
	{
		glAttachShader(ShaderID, mod);
	}

	glLinkProgram(ShaderID);
	CheckCompileErrors(ShaderID, "PROGRAM");
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void engine::graphics::ShaderObject::Bind()
{
	glUseProgram(ShaderID);
}

uint32 engine::graphics::ShaderObject::GetUniformLocation(string Name) const
{
	return glGetUniformLocation(ShaderID, Name.c_str());
}

void engine::graphics::ShaderObject::SetInt(uint32 UniformLocation, uint32 Value)
{
	glUniform1i(UniformLocation, Value);
}

void engine::graphics::ShaderObject::SetFloat(uint32 UniformLocation, float Value)
{
	glUniform1f(UniformLocation, Value);
}

void engine::graphics::ShaderObject::SetVec3(uint32 UniformLocation, Vector3 Value)
{
	glUniform3f(UniformLocation, Value.X, Value.Y, Value.Z);
}
