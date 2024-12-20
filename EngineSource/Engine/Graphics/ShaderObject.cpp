#include "ShaderObject.h"
#include <kui/Resource.h>
#include "ShaderLoader.h"
#include <Engine/Internal/OpenGL.h>
#include <Engine/Log.h>

bool engine::graphics::ShaderObject::CheckCompileErrors(uint32 ShaderID, string Type)
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
			return true;
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
			return true;
		}
	}
	return false;
}

engine::graphics::ShaderObject::ShaderObject(string VertexFile, string FragmentFile)
{
	Compile(VertexFile, FragmentFile);
}

engine::graphics::ShaderObject::~ShaderObject()
{
	if (Valid)
		Clear();
}

void engine::graphics::ShaderObject::ReCompile()
{
	Compile(VertexFile, FragmentFile);
}

void engine::graphics::ShaderObject::Compile(string VertexFile, string FragmentFile)
{
	this->VertexFile = VertexFile;
	this->FragmentFile = FragmentFile;
	std::vector<uint32> FragmentModules;

	auto Res = ShaderLoader::Current->Modules.ParseShader(FragmentFile);
	FragmentFile = Res.ResultSource;
	for (auto& mod : Res.DependencyModules)
	{
		FragmentModules.push_back(mod.ModuleObject);
	}
	Valid = true;
	unsigned int vertex, fragment;
	const char* VertexCString = VertexFile.c_str();
	const char* FragmentCString = FragmentFile.c_str();
	// vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, GLsizei(1), &VertexCString, nullptr);
	glCompileShader(vertex);
	if (CheckCompileErrors(vertex, "VERTEX"))
	{
		Valid = false;
		return;
	}
	// fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, GLsizei(1), &FragmentCString, nullptr);
	glCompileShader(fragment);
	if (CheckCompileErrors(fragment, "FRAGMENT"))
	{
		Valid = false;
		return;
	}
	// shader Program
	ShaderID = glCreateProgram();
	glAttachShader(ShaderID, vertex);
	glAttachShader(ShaderID, fragment);

	for (uint32 mod : FragmentModules)
	{
		glAttachShader(ShaderID, mod);
	}

	glLinkProgram(ShaderID);
	if (CheckCompileErrors(ShaderID, "PROGRAM"))
	{
		Valid = false;
		return;
	}
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void engine::graphics::ShaderObject::Bind()
{
	if (Valid)
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

void engine::graphics::ShaderObject::Clear()
{
	glDeleteProgram(ShaderID);
}
