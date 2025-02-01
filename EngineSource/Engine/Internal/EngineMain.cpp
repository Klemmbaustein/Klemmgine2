#include "Core/Types.h"
#include <Core/LaunchArgs.h>

#ifdef USE_ENGINEMAIN

int32 EngineMain(int argc, char** argv);

#if defined(WINDOWS) && defined(ENGINE_SUBSYSTEM_WIN32)
#include <Windows.h>

int WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd
)
{
	engine::launchArgs::SetArgs(__argc, __argv);
	return EngineMain(__argc, __argv);
}

#else

int main(int argc, char** argv)
{
	engine::launchArgs::SetArgs(argc, argv);
	return EngineMain(argc, argv);
}

#endif

#endif