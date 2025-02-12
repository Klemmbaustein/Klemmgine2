#pragma once
#include <Core/Types.h>
#include <Core/Log.h>

namespace engine::internal::platform
{
	void Execute(string Command);
	void Open(string File);

	void SetConsoleColor(Log::LogColor NewColor);

	void SetThreadName(string Name);
	string GetThreadName();
	
	struct SharedLibrary;

	SharedLibrary* LoadSharedLibrary(string Path);
	void* GetLibraryFunction(SharedLibrary* Library, string Name);
	void UnloadSharedLibrary(SharedLibrary* Library);
}