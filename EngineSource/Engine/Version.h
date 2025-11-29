#pragma once
#include <Core/Types.h>

namespace engine
{
	struct VersionInfo
	{
		/**
		 * @brief
		 * Gets a version info struct.
		 * @return
		 * A version info struct, containing all available version information.
		 */
		static VersionInfo Get();

		/// The version identifier. Usually follows semantic versioning (for example: 2.0.0)
		string VersionName;
		/// The platform the engine runs on, like "Windows" or "Linux"
		string Platform;
		/// The CPU architecture the engine runs on, like "AMD64" or "ARM64"
		string Architecture;
		/// The specific build identifier, usually containing a build type (like release, dev build, etc.) and a date.
		string Build;

		/**
		 * @brief
		 * Gets the short name of this version.
		 * @return
		 * A version string in the format Klemmgine-{VersionNumber}
		 *
		 * Example: Klemmgine 2.0.0
		 */
		string GetShortName() const;

		/**
		 * @brief
		 * Gets the build configuration of this version.
		 * @return
		 * A version string in the format {Platform} {Compiler} {Architecture}-{Configuration}
		 *
		 * Example: Windows MSVC AMD64-Debug
		 */
		string GetConfiguration() const;

		/**
		 * @brief
		 * Gets the display name of this version
		 * @return
		 * A version string in the format Klemmgine-{VersionNumber} ({Platform} {Compiler} {Architecture}-{Configuration}}
		 *
		 * Example: Klemmgine 2.0.0 (Windows MSVC AMD64-Debug)
		 */
		string GetDisplayName() const;

		/**
		 * @brief
		 * Gets the display name of this version, and a build ID name on the next line.
		 * @return
		 * A version string in the format Klemmgine-{VersionNumber} ({Platform} {Compiler} {Architecture}-{Configuration}}\nBuild ID: {BuildId}
		 *
		 * Example: Klemmgine 2.0.0 (Windows MSVC AMD64-Debug)\\n
		 *
		 * Build ID: Release-2.0.0 - DD/MM/YYYY
		 */
		string GetDisplayNameAndBuild() const;
	};
}