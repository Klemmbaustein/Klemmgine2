#pragma once

namespace engine::openGL
{
	enum class Version
	{
		/// The supported version is WebGL. Currently unused.
		WebGL,
		/// The current OpenGL version is at least 3.3
		GL330,
		/// The current OpenGL version is at least 4.3
		GL430,
	};

	extern Version VersionOverride;

	Version GetGLVersion();
}