#pragma once
#include <thread>
#include <vector>
#include <Core/Types.h>
#include <mutex>

namespace engine::platform
{
	struct Pipe
	{
		Pipe(string Command);
		~Pipe();

		void Write(string Message);
		void Update();

		static void ReadThread(Pipe* Target);
		static void WriteLogThread(Pipe* Target);

		void* PipeStdOutReadHandle = nullptr;
		void* PipeStdErrReadHandle = nullptr;
		void* PipeStdInWriteHandle = nullptr;

		bool Empty();
		void SetEmpty();

		int32 GetReturnValue();

		string GetErrorMessage();

		std::thread* PipeThread = nullptr;
		std::thread* LogThread = nullptr;

		std::vector<string> GetNewLines();

	private:

		void CloseProcess();

		void* ThreadHandle = nullptr;
		void* ProcessHandle = nullptr;

		bool IsEmpty = false;

		int32 ReturnValue = 0;

		string Received;
		string ErrorMessage;
		std::mutex MessagesMutex;
	};
}