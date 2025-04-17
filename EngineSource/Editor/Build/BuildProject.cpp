#include "BuildProject.h"
#include "BuildWin32.h"
#include <Core/Platform/Pipe.h>
#include <Core/Platform/Platform.h>
#include <iostream>

using namespace engine;
using namespace engine::editor;
using namespace engine::platform;

void engine::editor::BuildCurrentProject(BuildOptions Options)
{
#if WINDOWS
	switch (Options.Platform)
	{
	case BuildPlatform::Windows:
		BuildProjectWinX64(Options);
		break;
	case BuildPlatform::WindowsArm:
		break;
	case BuildPlatform::Linux:
		BuildProjectLinux(Options);
		break;
	default:
		break;
	}
#endif
}

bool engine::editor::BuildProjectExecuteCommand(string Command, BuildOptions& Options, BuildStage Stage)
{
#if WINDOWS
	Log::Info(Command);
	Pipe CmdPipe = Pipe(Command);

	while (!CmdPipe.Empty())
	{
		auto NewLines = CmdPipe.GetNewLines();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		for (const string& ln : NewLines)
		{
			Options.LogLineAdded(ln, Stage);
			std::cerr << (ln) << std::endl;
		}
	}

	if (CmdPipe.GetReturnValue() != 0)
	{
		string Error = CmdPipe.GetErrorMessage();

		if (Error.empty())
		{
			Options.LogLineAdded(str::Format("Error: Process returned %i", CmdPipe.GetReturnValue()), BuildStage::Failed);
		}
		else
		{
			Options.LogLineAdded(str::Format("Error: Process returned %s", Error.c_str()), BuildStage::Failed);
		}
		return false;
	}
#endif
	return true;
}
