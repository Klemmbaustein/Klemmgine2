#pragma once

namespace engine::openGL
{
	enum class Version
	{
		WebGL,
		GL330,
		GL430,
	};

	Version GetGLVersion();
}