#pragma once
#include <Core/Types.h>
#include <functional>

namespace engine::editor
{
	enum class BuildPlatform : uint8
	{
		Windows,
		WindowsArm,
		Linux,
	};

	enum class BuildStage : uint8
	{
		Configure,
		Compile,
		CreateBuild,
		Done,
		Failed,
	};


	struct BuildOptions
	{
		BuildPlatform Platform = BuildPlatform::Windows;

		bool CompressAssets = true;
		bool IncludeDevPlugins = true;
		bool MultiThreaded = true;

		string OutputPath;
		string BuildConfiguration;

		std::function<void(string, BuildStage)> LogLineAdded;
	};

	static BuildPlatform operator|(BuildPlatform a, BuildPlatform b)
	{
		return BuildPlatform(uint8(a) | uint8(b));
	}

	static bool operator&(BuildPlatform a, BuildPlatform b)
	{
		return bool(uint8(a) & uint8(b));
	}

	bool BuildProjectExecuteCommand(string Command, BuildOptions& Options, BuildStage Stage);
	void BuildCurrentProject(BuildOptions Options);

	void BuildProjectNative(BuildOptions& Options);
}