#if WINDOWS
#include "BuildWin32.h"
#include <thread>

using namespace engine;
using namespace engine::editor;

static string ConfigureArgs(const BuildOptions& Options)
{
	string OutArgs;

	if (Options.BuildConfiguration.empty())
	{
		OutArgs.append(" -DCMAKE_BUILD_TYPE=Debug");
	}
	else
	{
		OutArgs.append(" -DCMAKE_BUILD_TYPE=" + Options.BuildConfiguration);
	}

	return OutArgs;
}

static string CompileArgs(const BuildOptions& Options)
{
	string OutArgs;

	if (Options.MultiThreaded)
	{
		OutArgs.append(" --parallel -j " + std::to_string(std::thread::hardware_concurrency()));
	}

	if (Options.BuildConfiguration.empty())
	{
		OutArgs.append(" --config " + Options.BuildConfiguration);
	}
	else
	{
		OutArgs.append(" --config " + Options.BuildConfiguration);
	}

	return OutArgs;
}

void engine::editor::BuildProjectLinux(BuildOptions Options)
{
	string OutPath = Options.OutputPath;

	if (!BuildProjectExecuteCommand("wsl -- cmake -S . -B build/out/linux-x64/ -Wno-deprecated " + ConfigureArgs(Options), Options, BuildStage::Configure))
	{
		return;
	}

	if (!BuildProjectExecuteCommand("wsl -- cmake --build build/out/linux-x64/ --target ProjectCompiler " + CompileArgs(Options), Options, BuildStage::Compile))
	{
		return;
	}

	if (BuildProjectExecuteCommand("wsl -- build/out/linux-x64/bin/ProjectCompiler -out build/linux"
		+ ProjectBuildArgs(Options), Options, BuildStage::CreateBuild))
	{
		Options.LogLineAdded("", BuildStage::Done);
	}
}

void engine::editor::BuildProjectWinX64(BuildOptions Options)
{
	string OutPath = Options.OutputPath;

	if (!BuildProjectExecuteCommand("cmake -S . -B " + OutPath + "/out/win-x64/ -Wno-deprecated " + ConfigureArgs(Options), Options, BuildStage::Configure))
	{
		return;
	}

	if (!BuildProjectExecuteCommand("cmake --build " + OutPath + "/out/win-x64/ --target ProjectCompiler " + CompileArgs(Options), Options, BuildStage::Compile))
	{
		return;
	}

	if (BuildProjectExecuteCommand(
		str::ReplaceChar(OutPath, '/', '\\')
		+ "\\out\\win-x64\\bin\\ProjectCompiler.exe -out "
		+ OutPath
		+ "/win-x64"
		+ ProjectBuildArgs(Options),
		Options, BuildStage::CreateBuild))
	{
		Options.LogLineAdded("", BuildStage::Done);
	}
}

#endif
