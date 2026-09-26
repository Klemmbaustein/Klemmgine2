#pragma once
#include "GraphicsBackend.h"

namespace engine::graphics
{
	class OpenGLGraphicsBackend : public GraphicsBackend
	{
	public:

		enum class Version
		{
			/// The supported version is WebGL. Currently unused.
			WebGL,
			/// The current OpenGL version is at least 3.3
			GL330,
			/// The current OpenGL version is at least 4.3
			GL430,
		};

		OpenGLGraphicsBackend();

		// Inherited via GraphicsBackend
		Renderer* CreateRenderer() override;
		string GetBackendIdentifier() override;

	private:

		Version GLVersion;
		string OpenGLMode;
	};
}