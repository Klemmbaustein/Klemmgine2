#include "MainThread.h"
#include <mutex>
#include <cstdlib>

static std::vector<std::function<void()>> MainThreadFuncs;
static std::mutex MainThreadFuncsMutex;

void engine::thread::ExecuteOnMainThread(std::function<void()> Function)
{
	std::lock_guard g{ MainThreadFuncsMutex };
	MainThreadFuncs.push_back(Function);
}

void engine::thread::MainThreadUpdate()
{
	if (!IsMainThread)
		abort();
	std::lock_guard g{ MainThreadFuncsMutex };

	for (auto& i : MainThreadFuncs)
	{
		i();
	}
	MainThreadFuncs.clear();
}

thread_local bool engine::thread::IsMainThread = false;
