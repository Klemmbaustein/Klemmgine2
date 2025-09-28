#pragma once
#include <Engine/Subsystem/Subsystem.h>
#include <Engine/Plugins/PluginLoader.h>

namespace engine::subsystem
{
	class PluginSubsystem : public Subsystem
	{
	public:
		PluginSubsystem();
		~PluginSubsystem() override;

		void Update() override;
	};
}