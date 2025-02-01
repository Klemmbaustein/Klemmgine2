#pragma once
#include <Core/Types.h>

#define ENGINE_ASSERT(cond) if (!(cond)) ::engine::error::AssertFailure(# cond, "Assert failed", __FILE__, __LINE__)

#define ENGINE_ASSERT_MESSAGE(cond, message) if (!(cond)) ::engine::error::AssertFailure(# cond, message, __FILE__, __LINE__)

#define ENGINE_UNREACHABLE() ::engine::error::AssertFailure("Unreachable code", "Error", __FILE__, __LINE__)

namespace engine::error
{
	void AssertFailure(string ConditionString, string Message, string Path, size_t Line);
}