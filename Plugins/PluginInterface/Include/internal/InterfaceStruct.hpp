#pragma once
#include <functional>
#include <string>
#include <Core/Transform.h>
#include <Core/Vector.h>
#include "PluginCanvas.hpp"

namespace engine::plugin
{
#define STRUCT_MEMBER(name, ret, args, func) using name ## Fn = ret (*) args; name ## Fn name = nullptr;
#define STRUCT_MEMBER_CALL_DIRECT(name, ret, args, func) using name ## Fn = ret (*) args; name ## Fn name = nullptr;

	/**
	* @brief
	* The engine plugin interface struct, containing methods the plugin can call in the engine.
	*
	*
	*/

	class kuiUIBox;
	class PluginUICanvas;


	using CallbackFn = void(*)(void* UserData);

	struct EnginePluginInterface
	{
		/// The relative path to this plugin's asset files.
		const char* PluginPath = nullptr;
		/// The name of this plugin.
		const char* PluginName = nullptr;

		// Doxygen is really confused by this
		/// @cond
#include "InterfaceDefines.hpp"
		/// @endcond
	};

	using RegisterTypesFn = void(*)();
	using PluginLoadFn = void(*)(EnginePluginInterface*);
}