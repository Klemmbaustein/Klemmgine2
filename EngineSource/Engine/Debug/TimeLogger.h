#pragma once
#include <Core/Log.h>

namespace engine::debug
{

	struct TimeLogger
	{
		TimeLogger(string Measured, std::vector<Log::LogPrefix> Prefixes = {});
		~TimeLogger();

		void End();
	private:
		string Measured;
		std::vector<Log::LogPrefix> Prefixes;
		bool Ended = false;
		uint64 BeginTime = 0;
	};

}