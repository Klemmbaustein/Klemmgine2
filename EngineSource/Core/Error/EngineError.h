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
	[[nodiscard]]
	string GetThreadName();

	/**
	* @brief
	* Prints a stack trace, aborts the application.
	*
	* @param Message
	* The reason for aborting.
	*/
	[[noreturn]]
	void Abort(string Message = "Aborted");

	/**
	* @brief
	* The function that should be called when the application crashes.
	* By default, this is set to a function that prints the stack trace to the log,
	* but if the engine is compiled with the video subsystem it will be replaced with
	* a function showing a message box instead.
	*/
	extern std::function<void(string Error, string StackTrace) > OnErrorCallback;
}