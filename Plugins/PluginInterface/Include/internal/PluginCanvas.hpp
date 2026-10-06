#pragma once
#include <Core/Types.h>
#include <kui/UI/UICanvasBox.h>

namespace engine::plugin
{
	class PluginCanvasInterface
	{
	public:
		virtual void Update() = 0;
		virtual void Begin() = 0;

		virtual ~PluginCanvasInterface() = default;
		kui::UICanvasBox* UIObject = nullptr;
	};
}