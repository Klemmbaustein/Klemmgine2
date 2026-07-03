#pragma once
#include "GraphicsBackend.h"
#include <Engine/Subsystem/Subsystem.h>

namespace engine::graphics
{
	class OpenGLGraphicsBackend : public GraphicsBackend
	{
	public:

		OpenGLGraphicsBackend();

		// Inherited via GraphicsBackend
		Renderer* CreateRenderer() override;
		string GetBackendIdentifier() override;

	private:
		string OpenGLMode;
	};
}