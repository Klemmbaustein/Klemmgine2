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

		void LoadElement(string Name, string ElementSource, plugin::PluginCanvasInterface* Canvas);

		kui::UIBox* GetElement(string Name);
		kui::UIBox* CreateElement(string Name);

		kui::UIBox* GetRootBox();

	private:

		plugin::PluginCanvasInterface* Canvas = nullptr;
		kui::markup::UIDynMarkupBox* DynamicElement = nullptr;
		kui::markup::DynamicMarkupContext ctx;
	};
}