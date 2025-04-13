#pragma once
#include <Core/Types.h>

namespace engine
{
	struct VersionInfo
	{
		static VersionInfo Get();

		string VersionName;
		string Platform;
		string Architecture;
		string Build;

		string GetDisplayName() const;
		string GetDisplayNameAndBuild() const;
	};
}