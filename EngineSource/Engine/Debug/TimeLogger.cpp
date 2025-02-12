#include "TimeLogger.h"
#include <SDL3/SDL.h>

engine::debug::TimeLogger::TimeLogger(string Measured, std::vector<Log::LogPrefix> Prefixes)
{
	this->Measured = Measured;
	this->Prefixes = Prefixes;
	BeginTime = SDL_GetPerformanceCounter();
}

engine::debug::TimeLogger::~TimeLogger()
{
	End();
}

void engine::debug::TimeLogger::End()
{
	if (Ended)
		return;
	
	uint64 Difference = SDL_GetPerformanceCounter() - BeginTime;
	double DifferenceSeconds = double(Difference) / double(SDL_GetPerformanceFrequency());

	Log::Note(str::Format("%s (in %ims)", Measured.c_str(), int(DifferenceSeconds * 1000.0)), Prefixes);
	Ended = true;
}
