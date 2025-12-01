#pragma once
#include "ServerConnection.h"
#include <Editor/UI/Windows/IDialogWindow.h>

namespace engine::editor
{
	struct ConnectResult
	{
		bool Connect = false;
		ServerConnection* Connection = nullptr;
	};

	class ServerConnectDialog : IDialogWindow
	{
	public:

		ServerConnectDialog(std::function<void(ConnectResult)> SubmitResult);

		// Inherited via IDialogWindow
		void Begin() override;
		void Update() override;
		void Destroy() override;

	private:

		bool ConnectionAccepted = false;

		bool Accepted = false;

		void TryConnect();

		std::function<void(ConnectResult)> SubmitResult;
	};
}