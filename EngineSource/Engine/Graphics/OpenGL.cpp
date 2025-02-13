#include "OpenGL.h"
#include <Engine/Internal/OpenGL.h>

using namespace engine::openGL;
Version engine::openGL::VersionOverride = Version(-1);

Version engine::openGL::GetGLVersion()
{
	if (VersionOverride != Version(-1))
	{
		return VersionOverride;
	}
	if (GLEW_VERSION_4_3)
		return Version::GL430;
	return Version::GL330;
}
