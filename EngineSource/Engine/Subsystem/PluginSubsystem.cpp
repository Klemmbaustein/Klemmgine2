#include "PluginSubsystem.h"

engine::subsystem::PluginSubsystem::PluginSubsystem()
	: Subsystem("Plugin", Log::LogColor::Gray)
{
	plugin::Load(this);
}

engine::subsystem::PluginSubsystem::~PluginSubsystem()
{
}

void engine::subsystem::PluginSubsystem::Update()
{
	plugin::Update();
}
