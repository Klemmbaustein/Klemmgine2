#pragma once
#include <Engine/Types.h>
#include <mutex>

namespace engine
{
	class Log
	{
	public:
		enum class LogColor
		{
			Default,
			White,
			Red,
			Cyan,
			Blue,
			Yellow,
			Gray,
			Green,
		};

		struct LogPrefix
		{
			string Text;
			LogColor Color;
		};

		struct Message
		{
			std::vector<LogPrefix> Prefixes;
			string Message;
			LogColor Color;
		};

		static void PrintMsg(string Message, LogColor Color, std::vector<LogPrefix> Prefixes = {});

		static std::vector<Message> GetMessages();
		static size_t GetLogMessagesCount();
	private:
		static std::vector<Message> LogMessages;
		static std::mutex LogMutex;
	};
}