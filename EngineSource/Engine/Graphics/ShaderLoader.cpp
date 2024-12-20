#include "ShaderLoader.h"
#include <Engine/File/Resource.h>
using namespace engine::graphics;

ShaderLoader* ShaderLoader::Current = nullptr;
engine::graphics::ShaderLoader::ShaderLoader()
{
	Current = this;
}

engine::graphics::ShaderLoader::~ShaderLoader()
{
	for (auto& [_, Shader] : Loaded)
	{
		delete Shader;
	}
	Loaded.clear();
}

ShaderObject* engine::graphics::ShaderLoader::Get(string Vertex, string Fragment)
{
	auto Found = Loaded.find(Vertex + ";" + Fragment);

	if (Found != Loaded.end())
	{
		return Found->second;
	}

	ShaderObject* New = new ShaderObject(resource::GetTextFile(Vertex), resource::GetTextFile(Fragment));
	Loaded.insert({ Vertex + ";" + Fragment, New });

	return New;
}

void engine::graphics::ShaderLoader::ReloadAll()
{
	Modules.ScanModules();
	for (auto& i : Loaded)
	{
		i.second->ReCompile();
	}
}
