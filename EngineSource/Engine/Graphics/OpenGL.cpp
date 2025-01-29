#include "OpenGL.h"
#include <Engine/Internal/OpenGL.h>

using namespace engine::openGL;

Version engine::openGL::GetGLVersion()
{
	if (GLEW_VERSION_4_3)
		return Version::GL430;
	return Version::GL330;
}
