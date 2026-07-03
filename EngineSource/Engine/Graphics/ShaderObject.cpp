#include "ShaderObject.h"
#include "ShaderLoader.h"
#include <Engine/Graphics/VideoSubsystem.h>

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
	Uniforms.clear();
	Compile(VertexFile, FragmentFile);
}

void engine::graphics::ShaderObject::Compile(string VertexFile, string FragmentFile, string GeometryFile)
{
	this->VertexFile = VertexFile;
	this->FragmentFile = FragmentFile;
	this->GeometryFile = GeometryFile;
	std::vector<ShaderProgramObject*> ResultModules;

	auto VertexResult = ShaderLoader::Current->Modules.ParseShader(VertexFile, ShaderModule::ShaderType::Vertex);
	VertexFile = VertexResult.ResultSource;
	for (auto& mod : VertexResult.DependencyModules)
	{
		ResultModules.push_back(mod.Object);
		for (auto& dep : mod.Dependencies)
		{
			ResultModules.push_back(dep);
		}
	}

	auto FragmentResult = ShaderLoader::Current->Modules.ParseShader(FragmentFile, ShaderModule::ShaderType::Fragment);
	this->Unlit = FragmentResult.IsUnlit;
	FragmentFile = FragmentResult.ResultSource;
	for (auto& mod : FragmentResult.DependencyModules)
	{
		ResultModules.push_back(mod.Object);
		for (auto& dep : mod.Dependencies)
		{
			ResultModules.push_back(dep);
		}
	}

	auto Render = VideoSubsystem::Current->Renderer;

	Valid = true;

	ShaderProgramObject* Vertex = Render->CreateShaderProgramObject(VertexFile, ShaderProgramType::Vertex);
	ShaderProgramObject* Fragment = Render->CreateShaderProgramObject(FragmentFile, ShaderProgramType::Fragment);
	ShaderProgramObject* Geometry = nullptr;

	ResultModules.push_back(Vertex);
	ResultModules.push_back(Fragment);

	if (!GeometryFile.empty())
	{
		auto GeometryResult = ShaderLoader::Current->Modules.ParseShader(GeometryFile, ShaderModule::ShaderType::Geometry);
		GeometryFile = GeometryResult.ResultSource;
		const char* GeometryCString = GeometryFile.c_str();
		for (auto& mod : GeometryResult.DependencyModules)
		{
			ResultModules.push_back(mod.Object);
			for (auto& dep : mod.Dependencies)
			{
				ResultModules.push_back(dep);
			}
		}

		Geometry = Render->CreateShaderProgramObject(GeometryFile, ShaderProgramType::Geometry);
		ResultModules.push_back(Geometry);
	}

	for (auto& i : ResultModules)
	{
		if (!i)
		{
			Valid = false;
			return;
		}
	}

	Program = Render->LinkShaderProgram(ResultModules);

	if (!Program)
	{
		Valid = false;
		return;
	}

	ModelUniform = GetUniformLocation("u_model");
	Uniforms.clear();

	delete Vertex;
	delete Fragment;
	if (Geometry)
	{
		delete Geometry;
	}
}

void engine::graphics::ShaderObject::Bind()
{
	Program->Activate();
}

uint32 engine::graphics::ShaderObject::GetUniformBlockLocation(const string& Name)
{
	auto Found = UniformBlocks.find(Name);
	if (Found != UniformBlocks.end())
		return Found->second;

	uint32 Location = Program->GetUniformBlockLocation(Name.c_str());

	UniformBlocks.insert({ Name, Location });

	return Location;
}

uint32 engine::graphics::ShaderObject::GetUniformLocation(size_t NameHash, const char* Name) const
{
	auto Found = Uniforms.find(NameHash);
	if (Found != Uniforms.end())
		return Found->second;

	uint32 Location = Program->GetUniformLocation(Name);

	Uniforms.insert({ NameHash, Location });

	return Location;
}

void engine::graphics::ShaderObject::Clear()
{
	delete Program;
}
