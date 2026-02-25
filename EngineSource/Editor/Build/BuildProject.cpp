#include "BuildProject.h"
#include <Core/Platform/Pipe.h>
#include <Core/Platform/Platform.h>
#include <iostream>

#ifdef IS_SOURCELESS_BUILD
#include <filesystem>
#include <Editor/Editor.h>
#else
#include "BuildWin32.h"
#endif

using namespace engine;
using namespace engine::editor;
using namespace engine::platform;

void engine::editor::BuildCurrentProject(BuildOptions Options)
{
#ifndef IS_SOURCELESS_BUILD
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
#else
	string ExportPath = GetEditorPath() + "/Export/";

#if WINDOWS
	ExportPath = str::ReplaceChar(ExportPath, '/', '\\');
#endif

	try
	{

		if (std::filesystem::exists(Options.OutputPath))
		{
			std::filesystem::remove_all(Options.OutputPath);
		}
		std::filesystem::create_directories(Options.OutputPath);

		if (BuildProjectExecuteCommand("'" +
			ExportPath
			+ "ProjectCompiler' -out "
			+ Options.OutputPath
			+ ProjectBuildArgs(Options)
			+ " -binaryPath '" + ExportPath + "'",
			Options, BuildStage::CreateBuild))
		{
			Options.LogLineAdded("", BuildStage::Done);
		}
	}
	catch (std::filesystem::filesystem_error& e)
	{
		Options.LogLineAdded(e.what(), BuildStage::Failed);
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
