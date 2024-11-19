#pragma once
#include <functional>

namespace engine::thread
{
	void ExecuteOnMainThread(std::function<void()> Function);

	thread_local extern bool IsMainThread;

	void MainThreadUpdate();
}