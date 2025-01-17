#include "PluginSubsystem.h"

engine::subsystem::PluginSubsystem::PluginSubsystem()
	: Subsystem("Plugin", Log::LogColor::Gray)
{
	plugin::Load();
}

engine::subsystem::PluginSubsystem::~PluginSubsystem()
{
}

void engine::subsystem::PluginSubsystem::Update()
{
}
