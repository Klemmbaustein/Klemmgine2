#pragma once
#include <Core/Types.h>

namespace engine::plugin
{
	ENGINE_INTERFACE PluginCanvasInterface
	{
	public:
		virtual void Update() = 0;
		virtual void Begin() = 0;

		virtual ~PluginCanvasInterface() {};
		void* UIObject = nullptr;
	};
}