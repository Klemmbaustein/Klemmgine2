#include "Log.h"
#include <Engine/Internal/Platform.h>
#include <iostream>
std::vector<engine::Log::Message> engine::Log::LogMessages;
std::mutex engine::Log::LogMutex;

void engine::Log::PrintMsg(string Message, LogColor Color, std::vector<LogPrefix> Prefixes)
{
	using namespace internal::platform;

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
