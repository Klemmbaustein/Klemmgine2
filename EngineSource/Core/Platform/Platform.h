#pragma once
#include <Core/Types.h>
#include <Core/Log.h>
#include <vector>

namespace engine::internal::platform
{
	void Execute(string Command);

	void SetConsoleColor(Log::LogColor NewColor);

	void SetThreadName(string Name);
	string GetThreadName();
	
	struct SharedLibrary;

	SharedLibrary* LoadSharedLibrary(string Path);
	void* GetLibraryFunction(SharedLibrary* Library, string Name);
}