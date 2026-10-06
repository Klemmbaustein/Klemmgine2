#pragma once
#include <Engine/UI/UICanvas.h>
#include <Core/Types.h>
#include <kui/DynamicMarkup.h>
#include <PluginCanvas.hpp>

namespace engine::plugin
{
	class PluginUICanvas : public UICanvas
	{
	public:
		PluginUICanvas();
		virtual ~PluginUICanvas() override;

		virtual void Update() override;

		void LoadPluginCanvas(plugin::PluginCanvasInterface* Canvas);

	private:
		plugin::PluginCanvasInterface* Canvas = nullptr;
	};
}