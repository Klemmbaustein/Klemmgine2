#pragma once
#include "Types.h"
#include <functional>

namespace engine::subsystem
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
			subsystem::ConsoleSubsystem* Context = nullptr;
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
}