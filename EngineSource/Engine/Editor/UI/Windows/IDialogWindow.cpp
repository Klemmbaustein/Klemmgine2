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

	Vec2f ButtonSize = UIBox::PixelSizeToScreenSize(40, Popup);

	MainElement->SetMainBackgroundSize(Vec2f(2) - ButtonSize);
	MainElement->SetButtonBackgroundSize(ButtonSize);

	Background = MainElement->windowMain;
	ButtonBackground = MainElement->buttonBox;

	SetButtons(Options);
}

void engine::editor::IDialogWindow::SetButtons(std::vector<Option> Options)
{
	this->Options = Options;
	ButtonBackground->DeleteChildren();

	for (auto& i : this->Options)
	{
		auto NewButton = new DialogWindowButton();
		NewButton->btn->OnClicked = [this, &i]() {
			if (i.OnClicked)
				i.OnClicked();
			if (i.Close)
				Close();
			};
		NewButton->SetText(i.Name);
		ButtonBackground->AddChild(NewButton);
	}
}
