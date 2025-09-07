#include "Log.h"
#include "Log.h"
#include <Core/Platform/Platform.h>
#include <iostream>
std::vector<engine::Log::Message> engine::Log::LogMessages;
std::mutex engine::Log::LogMutex;

bool engine::Log::IsVerbose = false;

void engine::Log::PrintMsg(string Message, LogColor Color, std::vector<LogPrefix> Prefixes)
{
	if (Message.size() && Message[Message.size() - 1] == '\n')
	{
		Message.pop_back();
	}
	std::vector<string> Lines = str::Split(Message, "\n");
	for (auto& i : Lines)
	{
		PrintLine(i, Color, Prefixes);
	}
}

void engine::Log::Note(string Message, std::vector<LogPrefix> Prefixes)
{
	if (!IsVerbose)
		return;

	Prefixes.insert(Prefixes.begin(), LogPrefix{ .Text = "Note", .Color = Log::LogColor::Gray, });

	PrintMsg(Message, Log::LogColor::Gray, Prefixes);
}

void engine::Log::Info(string Message, std::vector<LogPrefix> Prefixes)
{
	Prefixes.insert(Prefixes.begin(), LogPrefix{ .Text = "Info", .Color = Log::LogColor::White, });

	PrintMsg(Message, Log::LogColor::White, Prefixes);
}

void engine::Log::Warn(string Message, std::vector<LogPrefix> Prefixes)
{
	Prefixes.insert(Prefixes.begin(), LogPrefix{ .Text = "Warn", .Color = Log::LogColor::Yellow, });

	PrintMsg(Message, Log::LogColor::Yellow, Prefixes);
}

void engine::Log::Error(string Message, std::vector<LogPrefix> Prefixes)
{
	Prefixes.insert(Prefixes.begin(), LogPrefix{ .Text = "Error", .Color = Log::LogColor::Red, });

	PrintMsg(Message, Log::LogColor::Red, Prefixes);
}

void engine::Log::Critical(string Message, std::vector<LogPrefix> Prefixes)
{
	Prefixes.insert(Prefixes.begin(), LogPrefix{ .Text = "Critical", .Color = Log::LogColor::Magenta, });

	PrintMsg(Message, Log::LogColor::Magenta, Prefixes);
}

std::vector<engine::Log::Message> engine::Log::GetMessages()
{
	std::lock_guard g{ LogMutex };

	std::vector<Message> MessagesCopy = LogMessages;
	return MessagesCopy;
}

size_t engine::Log::GetLogMessagesCount()
{
	std::lock_guard g{ LogMutex };
	return LogMessages.size();
}

void engine::Log::PrintLine(string Message, LogColor Color, const std::vector<LogPrefix>& Prefixes)
{
	using namespace platform;

	std::lock_guard g{ LogMutex };

	for (auto& i : Prefixes)
	{
		std::cout << "[";
		SetConsoleColor(i.Color);
		std::cout << i.Text;
		SetConsoleColor(LogColor::Default);
		std::cout << "]: ";
	}

	SetConsoleColor(Color);
	std::cout << Message;
	SetConsoleColor(LogColor::Default);
	std::cout << std::endl;
	LogMessages.push_back(Log::Message{
		.Prefixes = Prefixes,
		.Message = Message,
		.Color = Color,
		});
}
