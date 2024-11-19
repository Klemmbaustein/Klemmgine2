#pragma once
#include "ISubsystem.h"
#include <Engine/Console.h>
#include <unordered_map>

namespace engine::subsystem
{
	class ConsoleSubsystem : public ISubsystem
	{
	public:
		ConsoleSubsystem();

		void ExecuteCommand(const string& Command);

		void AddCommand(const console::Command& NewCommand);

	private:
		std::unordered_map<string, console::Command> Commands;
	};
}