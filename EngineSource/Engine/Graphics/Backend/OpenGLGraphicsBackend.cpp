#include "OpenGLGraphicsBackend.h"
#include <Engine/Graphics/Backend/OpenGLRenderer.h>
#include <Core/LaunchArgs.h>
#include <Core/Log.h>

using namespace engine;
using namespace engine::graphics;

engine::graphics::OpenGLGraphicsBackend::OpenGLGraphicsBackend()
{
	auto VersionArg = launchArgs::GetArg("gl");

	if (VersionArg.has_value() && VersionArg->size() == 1)
	{
		string VersionString = VersionArg->at(0).AsString();

		if (VersionString == "4.3")
		{
			GLVersion = Version::GL430;
		}
		else if (VersionString == "3.3")
		{
			GLVersion = Version::GL330;
		}
		else
		{
			Log::Info(str::Format("Unknown OpenGL version '%s' will be interpreted as default",
				VersionString.c_str()));
		}

		Log::Info(str::Format("OpenGL version set through command line: '%s'", VersionString.c_str()));
	}

	if (GLVersion < Version::GL430)
	{
		Log::Warn("Using OpenGL 3.3 instead of 4.3. Some graphics effects might not work.");
		OpenGLMode = "OpenGL 3.3";
	}
	else
	{
		OpenGLMode = "OpenGL 4.3+";
	}
}

Renderer* engine::graphics::OpenGLGraphicsBackend::CreateRenderer()
{
	return new OpenGLRenderer(GLVersion == Version::GL430);
}

string engine::graphics::OpenGLGraphicsBackend::GetBackendIdentifier()
{
	return OpenGLMode;
}
