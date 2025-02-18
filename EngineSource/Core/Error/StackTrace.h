#pragma once
#include <Core/Types.h>

namespace engine::error
{
	/**
	* @brief
	* Returns a string describing the stack trace up to this point.
	* 
	* This will use debug symbols if available or raw addresses, and will
	* demangle function names.
	*/
	string GetStackTrace();
}