#ifdef EDITOR
#include "IPopupWindow.h"
#include <Editor/UI/EditorUI.h>
#include <thread>
#include <Engine/Graphics/VideoSubsystem.h>
#include <Engine/Engine.h>

void engine::editor::IPopupWindow::Open()
{
	std::thread ThreadObject = std::thread(&IPopupWindow::WindowThread, this, Name, Size);
	ThreadObject.detach();
}

void engine::editor::IPopupWindow::SetTitle(string NewTitle)
{
	Popup->SetTitle(NewTitle);
}

engine::editor::IPopupWindow::IPopupWindow(string Name, kui::Vec2ui Size, bool Resizable, bool Closable)
{
	this->Name = Name;
	this->Size = Size;
	this->Resizable = Resizable;
	this->CanClose = Closable;
}

void engine::editor::IPopupWindow::Close()
{
	ShouldClose = true;
}

void engine::editor::IPopupWindow::WindowThread(string Name, kui::Vec2ui Size)
{
	using namespace kui;

	Window::WindowFlag Flags = Window::WindowFlag::None;

	if (!Resizable)
	{
		Flags = Flags | Window::WindowFlag::Popup;
	}
	else
	{
		Flags = Flags | Window::WindowFlag::Resizable;
	}

	Popup = new Window(Name, Flags, Window::POSITION_CENTERED, Size);
	Popup->DPIMultiplier = EditorUI::Instance->MainBackground->GetParentWindow()->DPIMultiplier;
	EditorUI::UpdateTheme(Popup, false);

	DefaultFont = new kui::Font(Engine::GetSubsystem<subsystem::VideoSubsystem>()->DefaultFontName);
	Popup->Markup.SetDefaultFont(DefaultFont);

	this->Begin();

	while (true)
	{
		if (!Popup->UpdateWindow() && CanClose)
		{
			break;
		}
		if (ShouldClose)
		{
			break;
		}
		this->Update();
	}
	this->Destroy();
	delete DefaultFont;
	delete Popup;
	delete this;
}
#endif