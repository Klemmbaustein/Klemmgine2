#include "Console.h"
#include <Engine/Subsystem/ConsoleSubsystem.h>
#include <Engine/Engine.h>

using namespace engine;
using namespace engine::subsystem;

void console::ExecuteCommand(string CmdString)
{
	ConsoleSubsystem* Console = Engine::GetSubsystem<ConsoleSubsystem>();
	if (Console)
	{
		Console->ExecuteCommand(CmdString);
	}
}