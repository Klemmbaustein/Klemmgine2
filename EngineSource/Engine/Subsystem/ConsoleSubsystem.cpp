#include "ConsoleSubsystem.h"
#include <sstream>

using namespace engine::console;

engine::subsystem::ConsoleSubsystem::ConsoleSubsystem()
	: ISubsystem("Console", Log::LogColor::White)
{
	AddCommand({
		Command{
			.Name = "sayHi",
			.Args = {},
			.OnCalled = [this](const Command::CallContext&) {
				Print("Hi!");
			}
		}
		});
}

void engine::subsystem::ConsoleSubsystem::ExecuteCommand(const string& Command)
{
	std::stringstream Stream;
	Stream << Command;

	std::vector<string> CommandString;

	while (true)
	{
		string New;
		Stream >> New;

		CommandString.push_back(New);

		if (Stream.eof())
			break;
	}

	if (CommandString.empty())
	{
		Print(str::Format("Empty command: '%s'", Command.c_str()), LogType::Error);
		return;
	}

	string FirstWord = CommandString[0];

	auto FoundCommand = Commands.find(FirstWord);

	if (FoundCommand != Commands.end())
	{
		Command::CallContext CommandContext = {
			.ProvidedArguments = CommandString
		};
		CommandContext.ProvidedArguments.erase(CommandContext.ProvidedArguments.begin());
		CommandContext.Context = this;

		FoundCommand->second.OnCalled(CommandContext);
		return;
	}
	Print(str::Format("Unknown command: %s", FirstWord.c_str()), LogType::Error);
}

void engine::subsystem::ConsoleSubsystem::AddCommand(const console::Command& NewCommand)
{
	Commands.insert({NewCommand.Name, NewCommand});
}
