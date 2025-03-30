#include "ConsoleSubsystem.h"
#include <Engine/Engine.h>
#include <sstream>
#include <Core/Error/EngineError.h>

using namespace engine::console;

engine::subsystem::ConsoleSubsystem::ConsoleSubsystem()
	: Subsystem("Console", Log::LogColor::White)
{
	AddCommand({
		Command{
			.Name = "crash",
			.Args = {},
			.OnCalled = [this](const Command::CallContext&) {
				error::Abort();
			}
		}
		});

	AddCommand({
	Command{
		.Name = "exit",
		.Args = {},
		.OnCalled = [this](const Command::CallContext&) {
			Engine::Instance->ShouldQuit = true;
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

		if (CommandContext.ProvidedArguments.size() != FoundCommand->second.Args.size())
		{
			Print(str::Format("Invalid number of arguments, got %i, expected %i",
				int(CommandContext.ProvidedArguments.size()), int(FoundCommand->second.Args.size())), LogType::Error);
			return;
		}

		FoundCommand->second.OnCalled(CommandContext);
		return;
	}
	Print(str::Format("Unknown command: %s", FirstWord.c_str()), LogType::Error);
}

void engine::subsystem::ConsoleSubsystem::AddCommand(const console::Command& NewCommand)
{
	Commands.insert({ NewCommand.Name, NewCommand });
}

void engine::subsystem::ConsoleSubsystem::RemoveCommand(const string& CommandName)
{
	if (Commands.contains(CommandName))
		Commands.erase(CommandName);
}
