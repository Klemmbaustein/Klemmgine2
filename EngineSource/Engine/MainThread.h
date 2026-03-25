#pragma once
#include <functional>
#include <Core/ThreadMessages.h>

/**
* @brief
* Engine threading utilities.
*/
namespace engine::thread
{
	/**
	* @brief
	* Adds this function to a queue of functions that should be executed on the main thread.
	*/
	void ExecuteOnMainThread(std::function<void()> Function);

	/**
	* @brief
	* True if this thread is the main thread, false if not.
	*/
	thread_local extern bool IsMainThread;
	extern ThreadMessagesRef MainThreadQueue;

	void InitializeMainThread();

	void MainThreadUpdate();
}