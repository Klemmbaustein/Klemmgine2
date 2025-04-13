#include "Version.h"

using namespace engine;
#define STR_INNER(x) # x
#define STR(x) STR_INNER(x)

VersionInfo VersionInfo::Get()
{
	return VersionInfo{
		.VersionName = "2.0.0",
#if WINDOWS
		.Platform = "Windows",
#elif LINUX
		.Platform = "Linux",
#endif
		.Architecture = STR(ENGINE_COMPILER_ID),
		.Build = "Dev build - " __DATE__
	};
}

string VersionInfo::GetDisplayName() const
{
	return "Klemmgine " + VersionName
		+ " (" + Platform + " " + Architecture + ")";
}

string engine::VersionInfo::GetDisplayNameAndBuild() const
{
	return GetDisplayName() + "\nBuild ID: " + Build;
}
