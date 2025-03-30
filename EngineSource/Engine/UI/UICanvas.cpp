#include "UICanvas.h"
#include <Engine/Scene.h>
#include <kui/Window.h>

using namespace engine;
using namespace kui;

std::vector<UICanvas*> UICanvas::ActiveCanvases;

engine::UICanvas::UICanvas()
{
	CanvasBox = new UIBox(true, -1);
	CanvasBox
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
