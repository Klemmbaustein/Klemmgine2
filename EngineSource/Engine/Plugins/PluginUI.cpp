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

void engine::plugin::PluginUICanvas::LoadElement(string Name, string ElementSource, plugin::PluginCanvasInterface* Canvas)
{
	ctx.LoadFiles({ kui::markup::MarkupFile{
		.Name = Name,
		.Content = ElementSource
	} });

	CanvasBox->DeleteChildren();

	DynamicElement = new kui::markup::UIDynMarkupBox(&ctx, Name);
	CanvasBox->AddChild(DynamicElement);
	this->Canvas = Canvas;
	Canvas->UIObject = this;
	Canvas->Begin();
	CanvasBox->UpdateElement();
	CanvasBox->RedrawElement();
}

kui::UIBox* engine::plugin::PluginUICanvas::GetElement(string Name)
{
	auto Found = DynamicElement->NamedChildren.find(Name);

	if (Found == DynamicElement->NamedChildren.end())
	{
		return nullptr;
	}
	return Found->second;
}

kui::UIBox* engine::plugin::PluginUICanvas::CreateElement(string Name)
{
	auto box = new kui::markup::UIDynMarkupBox(&ctx, Name);
	return box;
}

kui::UIBox* engine::plugin::PluginUICanvas::GetRootBox()
{
	return CanvasBox;
}
