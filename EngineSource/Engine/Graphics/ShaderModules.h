#pragma once
#include <Engine/Types.h>
#include <vector>
#include <map>

namespace engine::graphics
{
	class ShaderModule
	{
	public:
		string Name;
		uint32 ModuleObject = 0;
		std::vector<string> Exported;
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
			bool IsModule;
			ShaderModule ThisModule;
		};

		[[nodiscard]]
		Result ParseShader(const string& ShaderSource);

		void ScanModules();
	private:
		void LoadModule(const string& Source, ShaderModule& Info);
		std::map<string, ShaderModule> LoadedModules;
	};
}