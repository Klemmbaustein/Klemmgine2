#include "IPopupWindow.h"
#include <Editor/UI/EditorUI.h>
#include <Editor/Settings/EditorSettings.h>
#include <thread>
#include <Engine/Graphics/VideoSubsystem.h>
#include <Engine/Engine.h>

using namespace engine::editor;

static IPopupWindow* OpenPopup = nullptr;

void engine::editor::IPopupWindow::OnThemeChanged()
{
}

void engine::editor::IPopupWindow::Open()
{
	if (OpenPopup)
	{
		delete this;
		return;
	}

	OpenPopup = this;
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
	Popup->DPIMultiplier = EditorUI::Instance ? EditorUI::Instance->MainBackground->GetParentWindow()->DPIMultiplier : 1;
	EditorUI::UpdateTheme(Popup, false);

	bool ReloadTheme = false;

	Settings::GetInstance()->Interface.ListenToSetting(this, "theme", [&ReloadTheme](SerializedValue val) {
		ReloadTheme = true;
	});

	Settings::GetInstance()->Interface.ListenToSetting(this, "uiScale", [this](SerializedValue val) {
		Popup->DPIMultiplier = val.GetFloat();
	});
	auto Video = Engine::GetSubsystem<VideoSubsystem>();

	DefaultFont = new kui::Font(Video ? Video->DefaultFontName : "res:DefaultFont.ttf");
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

		if (ReloadTheme)
		{
			EditorUI::UpdateTheme(Popup, true);
			OnThemeChanged();
			ReloadTheme = false;
		}

		this->Update();
	}
	this->Destroy();
	OpenPopup = nullptr;
	delete DefaultFont;
	delete Popup;
	delete this;

	Settings::GetInstance()->Interface.RemoveListener(this);
}
