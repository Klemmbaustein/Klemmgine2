#pragma once
#include <Engine/Types.h>
#include <vector>
#include <map>
#include "Material.h"

namespace engine::graphics
{
	class ShaderModule
	{
	public:
		string Name;
		uint32 ModuleObject = 0;
		std::vector<string> Exported;
	};

	struct ShaderUniform
	{
		Material::Field::Type Type = Material::Field::Type::None;
		string Name;
		string DefaultValue;
	};

	class ShaderModuleLoader
	{
	public:
		ShaderModuleLoader();
		ShaderModuleLoader(const ShaderModuleLoader&) = delete;
		~ShaderModuleLoader();

		struct Result
		{
			string ResultSource;
			std::vector<ShaderModule> DependencyModules;
			std::vector<ShaderUniform> ShaderUniforms;
			bool IsModule;
			ShaderModule ThisModule;
		};

		[[nodiscard]]
		Result ParseShader(const string& ShaderSource);

		void ScanModules();
	private:
		void FreeModules();
		ShaderUniform ReadUniformDefinition(string DefinitionSource);
		void LoadModule(const string& Source, ShaderModule& Info);
		std::map<string, ShaderModule> LoadedModules;
	};
}