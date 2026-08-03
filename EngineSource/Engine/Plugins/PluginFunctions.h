#pragma once
#include <Core/Types.h>
#include <InterfaceStruct.hpp>
#include <Engine/Plugins/PluginInfo.h>

namespace engine
{
	class Scene;
	namespace plugin
	{
		class PluginSubsystem;
	}
}

namespace engine::plugin
{
	extern EnginePluginInterface PluginInterface;

	void InitializePlugin(PluginInfo* ToInitialize, PluginSubsystem* System, string PluginDir);
}