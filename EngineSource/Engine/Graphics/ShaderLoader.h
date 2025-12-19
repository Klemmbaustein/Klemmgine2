#pragma once
#include "ShaderObject.h"
#include "ShaderModules.h"
#include <unordered_map>

namespace engine::graphics
{
	class ShaderLoader
	{
		struct ShaderLoadData
		{
			ShaderObject* Object = nullptr;
			std::string VertexSource;
			std::string FragmentSource;
		};

		std::unordered_map<string, ShaderLoadData> Loaded;

	public:
		ShaderLoader();
		ShaderLoader(const ShaderLoader&) = delete;
		~ShaderLoader();

		ShaderModuleLoader Modules;

		ShaderObject* Get(string Vertex, string Fragment);
		void ReloadAll();

		static ShaderLoader* Current;
	};
}