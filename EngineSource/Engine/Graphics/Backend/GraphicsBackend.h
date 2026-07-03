#pragma once
#include <Engine/Graphics/Backend/Renderer.h>
#include <Core/Types.h>

namespace engine::graphics
{
	class GraphicsBackend
	{
	public:


		virtual Renderer* CreateRenderer() = 0;
		virtual string GetBackendIdentifier() = 0;
	};
}