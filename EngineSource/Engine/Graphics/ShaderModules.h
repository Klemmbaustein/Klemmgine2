#pragma once
#include <Core/Types.h>
#include <vector>
#include <map>
#include "Material.h"
#include <Engine/Graphics/Backend/Renderer.h>

namespace engine::graphics
{
	struct ExportedShaderItem
	{
		string Name;
		string Definition;
		bool IsUniform = false;

		std::strong_ordering operator<=>(const ExportedShaderItem& other) const
		{
			return Name <=> other.Name;
		}
	};

	class ShaderModule
	{
	public:
		string Name;
		ShaderProgramObject* Object = nullptr;

		std::vector<ShaderProgramObject*> Dependencies;

		std::set<ExportedShaderItem> Exported;
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
			bool IsModule = false;
			bool IsUnlit = false;
			ShaderModule ThisModule;
		};

		[[nodiscard]]
		Result ParseShader(const string& ShaderSource, ShaderModule::ShaderType Type);

		void ScanModules(Renderer* Render);
	private:
		void FreeModules();
		ShaderUniform ReadUniformDefinition(string DefinitionSource);
		void LoadModule(const string& Source, ShaderModule& Info);
		std::map<string, ShaderModule> LoadedModules;
		Renderer* Render = nullptr;
	};
}