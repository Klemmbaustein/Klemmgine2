#pragma once
#include <functional>
#include <string>
#include <Engine/Transform.h>
#include <Engine/Vector.h>

namespace engine::plugin
{
#define STRUCT_MEMBER(name, ret, args, func) using name ## Fn = ret (*) args; name ## Fn name = nullptr;
#define STRUCT_MEMBER_CALL_DIRECT(name, ret, args, func) using name ## Fn = ret (*) args; name ## Fn name = nullptr;

	struct EnginePluginInterface
	{
		const char* PluginPath = nullptr;
		const char* PluginName = nullptr;

#include "InterfaceDefines.hpp"
	};

	using RegisterTypesFn = void(*)();
	using PluginLoadFn = void(*)(EnginePluginInterface*);
}