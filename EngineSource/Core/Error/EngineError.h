#pragma once
#include "EngineAssert.h"
#include "StackTrace.h"
#include <Core/Types.h>
#include <functional>

/**
* @brief
* Error handing namespace.
* 
* @ingroup engine-core
*/
namespace engine::error
{
	/**
	* @brief
	* Initialized error handling for this thread.
	* 
	* @param ThreadName
	* The name of this thread displayed by debuggers and shown when the engine crashes.
	* 
	* @see GetThreadName
	*/
	void InitForThread(string ThreadName);

	/**
	* @brief
	* Gets the thread name of the current thread.
	* 
	* @see InitForThread
	*/
	string GetThreadName();

	/**
	* @brief
	* Prints a stack trace, aborts the application.
	* 
	* @param Message
	* The reason for aborting.
	*/
	void Abort(string Message = "Aborted");

	extern std::function<void(string Error, string StackTrace) > OnErrorCallback;
}