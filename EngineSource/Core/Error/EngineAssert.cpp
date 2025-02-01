#include "EngineAssert.h"
#include "EngineError.h"
#include <Core/File/FileUtil.h>

void engine::error::AssertFailure(string ConditionString, string Message, string Path, size_t Line)
{
	Abort(str::Format("%s: (%s) (%s, line %i)", Message.c_str(), ConditionString.c_str(), file::FileName(Path).c_str(), int(Line)));
}
