#include "PluginUI.h"

engine::plugin::PluginUICanvas::PluginUICanvas()
{
}
engine::plugin::PluginUICanvas::~PluginUICanvas()
{
	if (Canvas)
		delete Canvas;
}

void engine::plugin::PluginUICanvas::Update()
{
	if (Canvas)
		Canvas->Update();
}

void engine::plugin::PluginUICanvas::LoadPluginCanvas(plugin::PluginCanvasInterface* Canvas)
{
	CanvasBox->DeleteChildren();
	this->Canvas = Canvas;
	Canvas->UIObject = CanvasBox;
	Canvas->Begin();
	CanvasBox->UpdateElement();
	CanvasBox->RedrawElement();
}
