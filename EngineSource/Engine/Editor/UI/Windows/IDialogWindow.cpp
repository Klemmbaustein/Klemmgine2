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

	for (auto& i : Options)
	{
		
	}
}

void engine::editor::IDialogWindow::Update()
{
}

void engine::editor::IDialogWindow::Destroy()
{
}
