#include "IDialogWindow.h"
#include <Core/Log.h>
#include <Common.kui.hpp>
#include <DialogWindow.kui.hpp>
#include <Engine/MainThread.h>

using namespace kui;

engine::editor::IDialogWindow::IDialogWindow(string Title, std::vector<Option> Options, kui::Vec2ui Size)
	: IPopupWindow(Title, Size, false, false)
{
	this->Options = Options;
}

void engine::editor::IDialogWindow::Begin()
{
	auto MainElement = new DialogWindowMainElement();

	MainElement->SetMainBackgroundSize(UISize::Screen(UISize::Screen(2).GetScreen().Y - (40_px).GetScreen().Y));
	MainElement->SetButtonBackgroundSize(40_px);

	Background = MainElement->windowMain;
	ButtonBackground = MainElement->buttonBox;

	SetButtons(Options);

	Popup->Input.RegisterOnKeyDownCallback(Key::ESCAPE, this, [this] {
		if (Popup->Input.PollForText)
		{
			return;
		}

		for (auto& i : Options)
		{
			if (i.IsClose)
			{
				PressButton(i);
				return;
			}
		}
	});

	Popup->Input.RegisterOnKeyDownCallback(Key::RETURN, this, [this] {
		if (Popup->Input.PollForText && !Popup->UI.KeyboardFocusBox)
		{
			return;
		}

		for (auto& i : Options)
		{
			if (i.IsAccept)
			{
				PressButton(i);
				return;
			}
		}
	});
}

void engine::editor::IDialogWindow::SetButtons(std::vector<Option> Options)
{
	this->Options = Options;
	ButtonBackground->DeleteChildren();

	for (auto i = Options.rbegin(); i < Options.rend(); i++)
	{
		auto NewButton = new EditorButton();
		NewButton->btn->OnClicked = [this, Value = *i]() {
			PressButton(Value);
			};
		NewButton->SetText(i->Name);
		ButtonBackground->AddChild(NewButton);
	}
	ButtonBackground->RedrawElement();
}

void engine::editor::IDialogWindow::PressButton(const Option& o)
{
	if (o.OnClicked)
	{
		if (o.OnMainThread)
		{
			thread::ExecuteOnMainThread([this, ShouldClose = o.Close, OnClicked = o.OnClicked]()
			{
				OnClicked();
				if (ShouldClose)
				{
					Close();
				}
			});
		}
		else
		{
			o.OnClicked();
		}
	}
	if (o.Close && o.OnMainThread)
	{
		Close();
	}
}
