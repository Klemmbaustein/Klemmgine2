#pragma once
#include <Core/Types.h>
#include <mutex>
#include <vector>

namespace engine
{
	/**
	* @brief
	* Class for logging messages.
	*
	* Prints messages to the standard output (or in the future to a file)
	*
	* @todo Log to a file.
	*
	* @ingroup engine-core
	*/
	class Log
	{
	public:

		/**
		* @brief
		* A color used for logging messages.
		*
		* @see Log::PrintMsg()
		*/
		enum class LogColor
		{
			/// Gray log color. The default color used by the terminal.
			Default,
			/// White log color.
			White,
			/// Red log color.
			Red,
			/// Cyan log color.
			Cyan,
			/// Blue log color.
			Blue,
			/// Yellow log color.
			Yellow,
			/// Gray log color. The same as LogColor::Default.
			Gray,
			/// Magenta log color.
			Magenta,
			/// Green log color.
			Green,
		};

		/**
		* @brief
		* A prefix added to a log message.
		*
		* The prefix follows the format
		* `[<Text>]: `, where the `<Text>` portion will have the given color.
		*/
		struct LogPrefix
		{
			/// The text of the prefix. For example: "Info"
			string Text;
			/// The color of the prefix.
			LogColor Color = LogColor::Default;
		};

		struct Message
		{
			std::vector<LogPrefix> Prefixes;
			string Message;
			LogColor Color;
		};

#ifndef ENGINE_PLUGIN
		static bool IsVerbose;

		/**
		* @brief
		* Prints a message into the log, with the given color and the given prefixes.
		*
		* The message can contain multiple lines (separated by a newline character) and the prefix will be added for each line.
		*
		* @see LogColor
		* @see Log::Info()
		* @see Log::Warn()
		* @see Log::Error()
		*/
		static void PrintMsg(string Message, LogColor Color, std::vector<LogPrefix> Prefixes = {});

		/**
		* @brief
		* Prints an info message into the log.
		*
		* This function works like Log::PrintMsg(), but it only logs if Log::IsVerbose is true.
		* The [Note]: prefix is added and the log color is gray.
		*
		* @see Log::PrintMsg()
		* @ingroup engine-core
		*/
		static void Note(string Message, std::vector<LogPrefix> Prefixes = {});

		/**
		* @brief
		* Prints an info message into the log.
		*
		* This function works like Log::PrintMsg(), but the [Info]: prefix is added and the log color is white.
		*
		* @see Log::PrintMsg()
		* @ingroup engine-core
		*/
		static void Info(string Message, std::vector<LogPrefix> Prefixes = {});
		/**
		* @brief
		* Prints an warning message into the log.
		*
		* This function works like Log::PrintMsg(), but the [Warn]: prefix is added and the log color is yellow.
		*
		* @see Log::PrintMsg()
		* @ingroup engine-core
		*/
		static void Warn(string Message, std::vector<LogPrefix> Prefixes = {});
		/**
		* @brief
		* Prints an error message into the log.
		*
		* This function works like Log::PrintMsg(), but the [Error]: prefix is added and the log color is red.
		*
		* @see Log::PrintMsg()
		* @ingroup engine-core
		*/
		static void Error(string Message, std::vector<LogPrefix> Prefixes = {});
		/**
		* @brief
		* Prints an error message into the log.
		*
		* This function works like Log::PrintMsg(), but the [Critical]: prefix is added and the log color is magenta.
		*
		* @see Log::PrintMsg()
		* @ingroup engine-core
		*/
		static void Critical(string Message, std::vector<LogPrefix> Prefixes = {});

		static std::vector<Message> GetMessages();
		static size_t GetLogMessagesCount();
	private:
		static void PrintLine(string Message, LogColor Color, const std::vector<LogPrefix>& Prefixes);
		static std::vector<Message> LogMessages;
		static std::mutex LogMutex;
#endif
	};
}
