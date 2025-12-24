#include "MainThread.h"
#include <mutex>
#include <cstdlib>
#include <Core/Error/EngineError.h>

static std::vector<std::function<void()>> MainThreadFuncs;
static std::mutex MainThreadFuncsMutex;

void engine::thread::ExecuteOnMainThread(std::function<void()> Function)
{
	ENGINE_ASSERT(!IsMainThread);

	std::lock_guard g{ MainThreadFuncsMutex };
	MainThreadFuncs.push_back(Function);
}

void engine::thread::MainThreadUpdate()
{
	ENGINE_ASSERT(IsMainThread);

	MainThreadFuncsMutex.lock();
	std::vector Functions = MainThreadFuncs;
	MainThreadFuncs.clear();
	MainThreadFuncsMutex.unlock();

	for (auto& i : Functions)
	{
		i();
	}
}

thread_local bool engine::thread::IsMainThread = false;
