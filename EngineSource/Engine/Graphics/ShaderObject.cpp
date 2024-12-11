#include "ShaderObject.h"
#include <kui/Resource.h>
#include <GL/glew.h>
#include <iostream>

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
			std::cerr << ("Shader compilation error of type: "
				+ Type
				+ "\n"
				+ std::string(infoLog)) << std::endl;
		}
	}
	else
	{
		glGetProgramiv(ShaderID, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(ShaderID, 1024, NULL, infoLog);
			std::cerr << ("Shader linking error of type: "
				+ Type
				+ "\n"
				+ std::string(infoLog)) << std::endl;
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

	VertexCstringArray.reserve(FragmentFiles.size());
	for (auto& i : FragmentFiles)
	{
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
	glShaderSource(fragment, GLsizei(FragmentCstringArray.size()), FragmentCstringArray.data(), NULL);
	glCompileShader(fragment);
	CheckCompileErrors(fragment, "FRAGMENT");

	// shader Program
	ShaderID = glCreateProgram();
	glAttachShader(ShaderID, vertex);
	glAttachShader(ShaderID, fragment);
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
