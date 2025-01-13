#ifdef EDITOR
#include "IDialogWindow.h"
#include <Engine/Log.h>
#include <DialogWindow.kui.hpp>

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
}

void engine::editor::IDialogWindow::SetButtons(std::vector<Option> Options)
{
	this->Options = Options;
	ButtonBackground->DeleteChildren();

	for (auto i = Options.rbegin(); i < Options.rend(); i++)
	{
		auto NewButton = new DialogWindowButton();
		NewButton->btn->OnClicked = [this, Value = *i]() {
			if (Value.OnClicked)
				Value.OnClicked();
			if (Value.Close)
				Close();
			};
		NewButton->SetText(i->Name);
		ButtonBackground->AddChild(NewButton);
	}
}
#endif