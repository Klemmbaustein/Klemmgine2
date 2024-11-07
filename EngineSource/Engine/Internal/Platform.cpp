#include "Platform.h"
#ifdef WINDOWS
#include <Windows.h>

void engine::internal::platform::Init()
{
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
}

#else

void engine::internal::platform::Init()
{
}

#endif