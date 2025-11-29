#pragma once
#include <Core/Types.h>
#include <functional>

namespace engine
{
	class ConsoleSubsystem;
}

namespace engine::console
{
	struct Command
	{
		struct CallContext
		{
			std::vector<string> ProvidedArguments;
			ConsoleSubsystem* Context = nullptr;
		};

		struct Argument
		{
			string Name;

			bool Required = true;
		};

		string Name;
		std::vector<Argument> Args;
		std::function<void(const CallContext&)> OnCalled;
	};

	void ExecuteCommand(string CmdString);
}