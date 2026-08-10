#pragma once
#include "ShaderObject.h"
#include "ShaderModules.h"
#include <unordered_map>

namespace engine::graphics
{
	struct ShaderLoadData
	{
		ShaderObject* Object = nullptr;
		std::string VertexSource;
		std::string FragmentSource;
	};

	class ShaderLoader
	{
		std::unordered_map<string, ShaderLoadData> Loaded;

	public:
		ShaderLoader();
		ShaderLoader(const ShaderLoader&) = delete;
		~ShaderLoader();

		ShaderModuleLoader Modules;

		ShaderObject* Get(string Vertex, string Fragment);
		std::vector<ShaderLoadData> GetAllUsing(string Shader);
		void ReloadAll();

		static ShaderLoader* Current;
	};
}