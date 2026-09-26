#include "ShaderObject.h"
#include "ShaderLoader.h"

engine::graphics::ShaderObject::ShaderObject(string VertexFile, string FragmentFile,
	string GeometryFile, Renderer* Render)
{
	Compile(VertexFile, FragmentFile, GeometryFile, Render);
}

engine::graphics::ShaderObject::~ShaderObject()
{
	if (Valid)
		Clear();
}

void engine::graphics::ShaderObject::ReCompile(string VertexFile, string FragmentFile,
	Renderer* Render)
{
	Uniforms.clear();
	UniformBlocks.clear();
	Compile(VertexFile, FragmentFile, "", Render);
}

void engine::graphics::ShaderObject::Compile(string VertexFile, string FragmentFile, string GeometryFile,
	Renderer* Render)
{
	this->VertexFile = VertexFile;
	this->FragmentFile = FragmentFile;
	this->GeometryFile = GeometryFile;

	if (VertexFile.empty() || FragmentFile.empty())
	{
		this->Valid = false;
		return;
	}

	std::vector<ShaderProgramObject*> ResultModules;

	auto VertexResult = ShaderLoader::Current->Modules.ParseShader(VertexFile, ShaderModule::ShaderType::Vertex,
		Render);
	VertexFile = VertexResult.ResultSource;
	for (auto& mod : VertexResult.DependencyModules)
	{
		ResultModules.push_back(mod.Object);
		for (auto& dep : mod.Dependencies)
		{
			ResultModules.push_back(dep);
		}
	}

	auto FragmentResult = ShaderLoader::Current->Modules.ParseShader(FragmentFile, ShaderModule::ShaderType::Fragment,
		Render);
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

	Valid = true;

	ShaderProgramObject* Vertex = Render->CreateShaderProgramObject(VertexFile, ShaderProgramType::Vertex);
	ShaderProgramObject* Fragment = Render->CreateShaderProgramObject(FragmentFile, ShaderProgramType::Fragment);
	ShaderProgramObject* Geometry = nullptr;

	ResultModules.push_back(Vertex);
	ResultModules.push_back(Fragment);

	if (!GeometryFile.empty())
	{
		auto GeometryResult = ShaderLoader::Current->Modules.ParseShader(GeometryFile, ShaderModule::ShaderType::Geometry,
			Render);
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
	if (Program)
	{
		Program->Activate();
	}
}

uint32 engine::graphics::ShaderObject::GetUniformBlockLocation(const string& Name)
{
	if (!Program)
	{
		return 0;
	}
	auto Found = UniformBlocks.find(Name);
	if (Found != UniformBlocks.end())
		return Found->second;

	uint32 Location = Program->GetUniformBlockLocation(Name.c_str());

	UniformBlocks.insert({ Name, Location });

	return Location;
}

uint32 engine::graphics::ShaderObject::GetUniformLocation(size_t NameHash, const char* Name) const
{
	if (!Program)
	{
		return 0;
	}

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
