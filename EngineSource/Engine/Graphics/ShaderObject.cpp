#include "ShaderObject.h"
#include "ShaderObject.h"
#include <kui/Resource.h>
#include "ShaderLoader.h"
#include <Engine/Internal/OpenGL.h>
#include <Core/Log.h>

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

engine::graphics::ShaderObject::ShaderObject(string VertexFile, string FragmentFile, string GeometryFile)
{
	Compile(VertexFile, FragmentFile, GeometryFile);
}

engine::graphics::ShaderObject::~ShaderObject()
{
	if (Valid)
		Clear();
}

void engine::graphics::ShaderObject::ReCompile(string VertexFile, string FragmentFile)
{
	Compile(VertexFile, FragmentFile);
}

void engine::graphics::ShaderObject::Compile(string VertexFile, string FragmentFile, string GeometryFile)
{
	this->VertexFile = VertexFile;
	this->FragmentFile = FragmentFile;
	this->GeometryFile = GeometryFile;
	std::vector<uint32> VertexModules;
	std::vector<uint32> FragmentModules;
	std::vector<uint32> GeometryModules;

	auto VertexResult = ShaderLoader::Current->Modules.ParseShader(VertexFile, ShaderModule::ShaderType::Vertex);
	VertexFile = VertexResult.ResultSource;
	for (auto& mod : VertexResult.DependencyModules)
	{
		VertexModules.push_back(mod.ModuleObject);
	}

	auto FragmentResult = ShaderLoader::Current->Modules.ParseShader(FragmentFile, ShaderModule::ShaderType::Fragment);
	FragmentFile = FragmentResult.ResultSource;
	for (auto& mod : FragmentResult.DependencyModules)
	{
		FragmentModules.push_back(mod.ModuleObject);
	}

	Valid = true;
	unsigned int vertex = 0, fragment = 0, geometry = 0;
	const char* VertexCString = VertexFile.c_str();
	const char* FragmentCString = FragmentFile.c_str();

	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, GLsizei(1), &VertexCString, nullptr);
	glCompileShader(vertex);
	if (CheckCompileErrors(vertex, "VERTEX"))
	{
		Valid = false;
		return;
	}

	if (!GeometryFile.empty())
	{
		auto GeometryResult = ShaderLoader::Current->Modules.ParseShader(GeometryFile, ShaderModule::ShaderType::Geometry);
		GeometryFile = GeometryResult.ResultSource;
		const char* GeometryCString = GeometryFile.c_str();
		for (auto& mod : GeometryResult.DependencyModules)
		{
			GeometryModules.push_back(mod.ModuleObject);
		}

		geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, GLsizei(1), &GeometryCString, nullptr);
		glCompileShader(geometry);
		if (CheckCompileErrors(geometry, "GEOMETRY"))
		{
			Valid = false;
			return;
		}
	}

	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, GLsizei(1), &FragmentCString, nullptr);
	glCompileShader(fragment);
	if (CheckCompileErrors(fragment, "FRAGMENT"))
	{
		Valid = false;
		return;
	}

	ShaderID = glCreateProgram();
	glAttachShader(ShaderID, vertex);
	glAttachShader(ShaderID, fragment);
	if (!GeometryFile.empty())
	{
		glAttachShader(ShaderID, geometry);
	}
	for (uint32 mod : FragmentModules)
	{
		glAttachShader(ShaderID, mod);
	}
	for (uint32 mod : VertexModules)
	{
		glAttachShader(ShaderID, mod);
	}
	for (uint32 mod : GeometryModules)
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
	if (!GeometryFile.empty())
	{
		glDeleteShader(geometry);
	}

	ModelUniform = glGetUniformLocation(ShaderID, "u_model");
	Uniforms.clear();
}

void engine::graphics::ShaderObject::Bind()
{
	if (Valid)
		glUseProgram(ShaderID);
}

uint32 engine::graphics::ShaderObject::GetUniformLocation(string Name) const
{
	auto Found = Uniforms.find(Name);
	if (Found != Uniforms.end())
		return Found->second;

	uint32 Location = glGetUniformLocation(ShaderID, Name.c_str());

	Uniforms.insert({Name, Location});

	return Location;
}

void engine::graphics::ShaderObject::SetInt(uint32 UniformLocation, int32 Value)
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

void engine::graphics::ShaderObject::SetVec2(uint32 UniformLocation, Vector2 Value)
{
	glUniform2f(UniformLocation, Value.X, Value.Y);
}

void engine::graphics::ShaderObject::SetTransform(uint32 UniformLocation, const Transform& Value)
{
	glUniformMatrix4fv(UniformLocation, 1, false, &Value.Matrix[0][0]);
}

void engine::graphics::ShaderObject::Clear()
{
	glDeleteProgram(ShaderID);
	ShaderID = 0;
}
