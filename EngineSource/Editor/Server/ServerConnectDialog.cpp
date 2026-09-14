#include "ServerConnectDialog.h"
#include <kui/Window.h>
#include <Engine/MainThread.h>
#include <Editor/UI/EditorUI.h>
using namespace engine::editor;
using namespace kui;

engine::editor::ServerConnectDialog::ServerConnectDialog(std::function<void(ConnectResult)> SubmitResult)
	: IDialogWindow("Connect to server", {
	Option{
	.Name = "Connect",
	.OnClicked = [this]() {
			TryConnect();
		},
	.Close = false,
	},
	Option{
	.Name = "Cancel",
	.Close = true,
	}, }, Vec2ui(400, 300))
{
	this->SubmitResult = SubmitResult;
	this->Open();
}

void engine::editor::ServerConnectDialog::Begin()
{
	IDialogWindow::Begin();

	Background->SetHorizontal(false);

	Url = new EditorTextField();
	Url->SetHintText("Server URL");
	Url->SetImage(EditorUI::Asset("Rename.png"));
	Background->AddChild(Url);

	Password = new EditorTextField();
	Password->SetHintText("Password");
	Password->SetImage(EditorUI::Asset("Rename.png"));
	Password->field->TransformDisplayText = [] (const std::string& Text) {
		std::string r;
		r.resize(Text.size(), '*');
		return r;
	};
	Background->AddChild(Password);

}

void engine::editor::ServerConnectDialog::Update()
{
}

void engine::editor::ServerConnectDialog::Destroy()
{
}

void engine::editor::ServerConnectDialog::TryConnect()
{
	auto c = new ServerConnection(Url->field->GetText(), Password->field->GetText());

	c->OnConnectionAcceptDeny = [this, c](bool Accept) {
		ConnectionAccepted = Accept;

		if (!ConnectionAccepted)
		{
			delete c;
		}
		else
		{
			thread::ExecuteOnMainThread(std::bind(this->SubmitResult, ConnectResult{
				.Connect = ConnectionAccepted,
				.Connection = c,
			}));
		}
		this->Close();
	};

	this->SetButtons({});
}
