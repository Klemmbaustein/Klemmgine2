#pragma once
#include <Core/Types.h>
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
		enum class ShaderType
		{
			Vertex,
			Fragment,
			Geometry
		};

		ShaderType Type = ShaderType::Vertex;
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
		Result ParseShader(const string& ShaderSource, ShaderModule::ShaderType Type);

		void ScanModules();
	private:
		void FreeModules();
		ShaderUniform ReadUniformDefinition(string DefinitionSource);
		void LoadModule(const string& Source, ShaderModule& Info);
		std::map<string, ShaderModule> LoadedModules;
	};
}