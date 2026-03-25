#include "MainThread.h"
#include <Core/Error/EngineError.h>

engine::thread::ThreadMessagesRef engine::thread::MainThreadQueue;
thread_local bool engine::thread::IsMainThread = false;

void engine::thread::ExecuteOnMainThread(std::function<void()> Function)
{
	ENGINE_ASSERT(!IsMainThread);

	MainThreadQueue->Run(Function);
}

void engine::thread::InitializeMainThread()
{
	IsMainThread = true;
	if (!MainThreadQueue)
	{
		MainThreadQueue = std::make_shared<ThreadMessageQueue>();
	}
}

void engine::thread::MainThreadUpdate()
{
	ENGINE_ASSERT(IsMainThread);

	MainThreadQueue->Update();
}

