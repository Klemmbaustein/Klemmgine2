#include "UICanvas.h"
#include <kui/Window.h>
#ifdef EDITOR
#include <Editor/UI/Panels/Viewport.h>
#endif

using namespace engine;
using namespace kui;

std::vector<UICanvas*> UICanvas::ActiveCanvases;

engine::UICanvas::UICanvas()
{
	CanvasBox = new UICanvasBox(0, 0);
	RegisterSelf(this);
	CanvasBox->UpdateElement();
}

engine::UICanvas::~UICanvas()
{
	for (auto i = ActiveCanvases.begin(); i < ActiveCanvases.end(); i++)
	{
		if (*i == this)
		{
			ActiveCanvases.erase(i);
			break;
		}
	}
	if (CanvasBox)
	{
		delete CanvasBox;
	}
}

void engine::UICanvas::Update()
{
}

void engine::UICanvas::UpdateAll()
{
	auto CanvasCopy = ActiveCanvases;

	for (auto& i : CanvasCopy)
	{
#ifdef EDITOR
		if (editor::Viewport::Current)
		{
			auto ViewBackground = editor::Viewport::Current->ViewportBackground;
			i->CanvasBox->IsVisible = editor::Viewport::Current->GetShowUI();
			i->CanvasBox
				->SetPosition(ViewBackground->GetPosition())
				->SetSize(ViewBackground->GetMinSize());

		}
#endif
		if (Engine::IsPaused)
		{
			continue;
		}

		i->Update();
	}
}

void engine::UICanvas::ClearAll()
{
	Window::GetActiveWindow()->UI.ButtonEvents.clear();
	std::vector CanvasCopy = ActiveCanvases;
	for (auto& i : CanvasCopy)
	{
		delete i;
	}
	ActiveCanvases.clear();
}

void engine::UICanvas::RegisterSelf(UICanvas* This)
{
	ActiveCanvases.push_back(This);
#ifdef EDITOR
	if (editor::Viewport::Current)
	{
		auto ViewBackground = editor::Viewport::Current->ViewportBackground;
		This->CanvasBox->IsVisible = editor::Viewport::Current->GetShowUI();
		This->CanvasBox
			->SetPosition(ViewBackground->GetPosition())
			->SetSize(ViewBackground->GetMinSize());

		return;
	}
#endif
	This->CanvasBox
		->SetPosition(-1)
		->SetMinSize(2)
		->SetMaxSize(2);

}
