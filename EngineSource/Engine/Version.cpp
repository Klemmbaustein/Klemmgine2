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
#ifdef ENGINE_BUILD_ID
		.Build = ENGINE_BUILD_ID + string(__DATE__),
#else
		.Build = "Dev build - " __DATE__,
#endif
	};
}

string engine::VersionInfo::GetShortName() const
{
	return "Klemmgine " + VersionName;
}

string engine::VersionInfo::GetConfiguration() const
{
	return Platform + " " + Architecture;
}

string VersionInfo::GetDisplayName() const
{
	return GetShortName() + " (" + GetConfiguration() + ")";
}

string engine::VersionInfo::GetDisplayNameAndBuild() const
{
	return GetDisplayName() + "\nBuild ID: " + Build;
}
