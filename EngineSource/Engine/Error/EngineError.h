#pragma once
#include "EngineAssert.h"
#include "StackTrace.h"
#include <Engine/Types.h>

namespace engine::error
{
	void InitForThread(string ThreadName);

	string GetThreadName();

	void Abort(string Message = "Aborted");
}