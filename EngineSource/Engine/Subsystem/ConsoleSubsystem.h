#pragma once
#include "Subsystem.h"
#include <Engine/Console.h>
#include <unordered_map>
#include <thread>
#include <condition_variable>

namespace engine
{
	class ConsoleSubsystem : public subsystem::Subsystem
	{
	public:
		ConsoleSubsystem();
		~ConsoleSubsystem();

		void ExecuteCommand(const string& Command);

		void Update() override;

		void AddCommand(const console::Command& NewCommand);
		void RemoveCommand(const string& CommandName);

		static void FlushLogs();

		static bool WriteLogs;

	private:
		static std::condition_variable LogWriteCondition;
		static std::mutex LogWriteMutex;

		static void LogWriteFunction();

		std::unordered_map<string, console::Command> Commands;
	};
}