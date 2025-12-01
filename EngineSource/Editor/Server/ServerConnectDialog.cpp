#include "ServerConnectDialog.h"
#include <kui/Window.h>
#include <Common.kui.hpp>
#include <condition_variable>
#include <Engine/MainThread.h>
#include <mutex>
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
}

void engine::editor::ServerConnectDialog::Update()
{
}

void engine::editor::ServerConnectDialog::Destroy()
{
}

void engine::editor::ServerConnectDialog::TryConnect()
{
	auto c = new ServerConnection("localhost:5000");

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
