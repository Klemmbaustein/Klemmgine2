#include "UICanvas.h"
#include <Engine/Scene.h>
#include <kui/Window.h>
#ifdef EDITOR
#include <Editor/UI/Panels/Viewport.h>
#endif

using namespace engine;
using namespace kui;

std::vector<UICanvas*> UICanvas::ActiveCanvases;

engine::UICanvas::UICanvas()
{
	CanvasBox = new UIBox(true, -1);

#ifdef EDITOR
	if (editor::Viewport::Current)
	{
		auto ViewBackground = editor::Viewport::Current->ViewportBackground;

		CanvasBox
			->SetPosition(ViewBackground->GetPosition())
			->SetMinSize(ViewBackground->GetMinSize())
			->SetMaxSize(ViewBackground->GetMinSize());
		return;
	}
#endif
	CanvasBox
		->SetPosition(-1)
		->SetMinSize(2)
		->SetMaxSize(2);
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
	delete CanvasBox;
}

void engine::UICanvas::Update()
{
}

void engine::UICanvas::UpdateAll()
{
	for (auto& i : ActiveCanvases)
	{
		i->Update();
	}
}

void engine::UICanvas::ClearAll()
{
	Window::GetActiveWindow()->UI.ButtonEvents.clear();
	auto CanvasCopy = ActiveCanvases;
	for (auto& i : CanvasCopy)
	{
		delete i;
	}
	ActiveCanvases.clear();
}

void engine::UICanvas::RegisterSelf(UICanvas* This)
{
	ActiveCanvases.push_back(This);
}
