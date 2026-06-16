#include "ConsoleSubsystem.h"
#include <Engine/Engine.h>
#include <sstream>
#include <Core/Error/EngineError.h>
#include <Core/Log.h>
#include <Core/LaunchArgs.h>
#include <Editor/Editor.h>
#include <fstream>
#include <chrono>
#include <filesystem>
#include <Core/Error/EngineError.h>

using namespace engine;
using namespace engine::console;

bool ConsoleSubsystem::WriteLogs = false;
std::condition_variable ConsoleSubsystem::LogWriteCondition;
std::mutex ConsoleSubsystem::LogWriteMutex;
static bool StopLogWrite = false;
std::thread LogWriteThread;

engine::ConsoleSubsystem::ConsoleSubsystem()
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

	AddCommand({
	Command{
		.Name = "sys.unload",
		.Args = {Command::Argument("name")},
		.OnCalled = [this](const Command::CallContext& ctx) {
			Engine::GetSubsystemByName(ctx.ProvidedArguments.at(0))->Unload();
		}
		}
		});

	WriteLogs = launchArgs::GetArg("writeLogs").has_value();

#ifndef EDITOR
	if (WriteLogs)
#endif
	{
		LogWriteThread = std::thread(LogWriteFunction);
		WriteLogs = true;
	}
}

engine::ConsoleSubsystem::~ConsoleSubsystem()
{
	FlushLogs();
}

void engine::ConsoleSubsystem::FlushLogs()
{
	if (WriteLogs)
	{
		{
			std::unique_lock g{ LogWriteMutex };
			StopLogWrite = true;
			LogWriteCondition.notify_all();
		}
		LogWriteThread.join();
	}
}

void engine::ConsoleSubsystem::ExecuteCommand(const string& Command)
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

void engine::ConsoleSubsystem::Update()
{
	LogWriteCondition.notify_one();
}

void engine::ConsoleSubsystem::AddCommand(const console::Command& NewCommand)
{
	Commands.insert({ NewCommand.Name, NewCommand });
}

void engine::ConsoleSubsystem::RemoveCommand(const string& CommandName)
{
	if (Commands.contains(CommandName))
		Commands.erase(CommandName);
}

void engine::ConsoleSubsystem::LogWriteFunction()
{
	error::InitForThread("Log write thread");

	size_t LastMessages = 0;

	std::filesystem::create_directory("Logs/");

	auto TimeValue = std::chrono::system_clock::now();

	string s = std::format("Logs/Engine-{:%Y-%m-%d-%H-%M-%OS}.txt", TimeValue);

	std::ofstream LogsStream = std::ofstream(s);

	do {
		std::unique_lock g{ LogWriteMutex };

		LogWriteCondition.wait(g);

		if (Log::GetLogMessagesCount() != LastMessages)
		{
			std::vector Messages = Log::GetMessages();
			if (Messages.size() < LastMessages)
			{
				LastMessages = 0;
			}

			for (size_t i = LastMessages; i < Messages.size(); i++)
			{
				auto MessageTimestamp = std::chrono::system_clock::now();

				LogsStream << std::format("{:%H:%M:%OS} > ", MessageTimestamp);
				for (auto& pref : Messages[i].Prefixes)
				{
					LogsStream << "[" << pref.Text << "]: ";
				}
				LogsStream << Messages[i].Message << std::endl;
			}
			LastMessages = Messages.size();
		}

	} while (!StopLogWrite);
	LogsStream.close();
}
