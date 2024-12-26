#pragma once
#include "Subsystem.h"
#include <Engine/Console.h>
#include <unordered_map>

namespace engine::subsystem
{
	class ConsoleSubsystem : public Subsystem
	{
	public:
		ConsoleSubsystem();

		void ExecuteCommand(const string& Command);

		void AddCommand(const console::Command& NewCommand);
		void RemoveCommand(const string& CommandName);

	private:
		std::unordered_map<string, console::Command> Commands;
	};
}