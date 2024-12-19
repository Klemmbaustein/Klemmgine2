#pragma once
#include "ShaderObject.h"
#include "ShaderModules.h"
#include <unordered_map>

namespace engine::graphics
{
	class ShaderLoader
	{
		std::unordered_map<string, ShaderObject*> Loaded;

	public:
		ShaderLoader();
		ShaderLoader(const ShaderLoader&) = delete;
		~ShaderLoader();

		ShaderModuleLoader Modules;

		ShaderObject* Get(string Vertex, string Fragment);

		static ShaderLoader* Current;
	};
}