#ifdef EDITOR
#include "MessageWindow.h"
#include <kui/UI/UIText.h>
#include <Engine/MainThread.h>
#include <Engine/Editor/UI/EditorUI.h>
using namespace kui;

engine::editor::MessageWindow::MessageWindow(string Message, std::function<void()> OnAccepted)
	: MessageWindow(Message, "Klemmgine 2", OnAccepted)
{
}

engine::editor::MessageWindow::MessageWindow(string Message, string Title, std::function<void()> OnAccepted)
	: MessageWindow(Message, Title, { Option{
	.Name = "Ok",
	.OnClicked = [OnAccepted, this]() {
			if (OnAccepted)
				OnAccepted();
		}} })
{
}

engine::editor::MessageWindow::MessageWindow(string Message, string Title, std::vector<Option> Options)
	: IDialogWindow(Title, Options, kui::Vec2i(400, 220))
{
	this->Message = Message;
	Open();
}

void engine::editor::MessageWindow::Begin()
{
	IDialogWindow::Begin();
	Background->AddChild((new UIText(11_px, EditorUI::Theme.Text, Message, DefaultFont))
		->SetWrapEnabled(true, 370_px)
		->SetPadding(10_px));
}

void engine::editor::MessageWindow::Update()
{
}

void engine::editor::MessageWindow::Destroy()
{
}
#endif