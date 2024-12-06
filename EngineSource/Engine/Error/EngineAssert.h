#pragma once
#include <Engine/Types.h>

#define ENGINE_ASSERT(cond) if (!(cond)) ::engine::error::AssertFailure(# cond, "Assert failed", __FILE__, __LINE__)

#define ENGINE_ASSERT_MESSAGE(cond, message) if (!(cond)) ::engine::error::AssertFailure(# cond, message)

namespace engine::error
{
	void AssertFailure(string ConditionString, string Message, string Path, size_t Line);
}