#pragma once
#include "ServerConnection.h"
#include <Editor/UI/Windows/DialogWindow.h>
#include <Common.kui.hpp>

namespace engine::editor
{
	struct ConnectResult
	{
		bool Connect = false;
		ServerConnection* Connection = nullptr;
	};

	class ServerConnectDialog : DialogWindow
	{
	public:

		ServerConnectDialog(std::function<void(ConnectResult)> SubmitResult);

		// Inherited via DialogWindow
		void Begin() override;
		void Update() override;
		void Destroy() override;

	private:
		bool ConnectionAccepted = false;

		bool Accepted = false;

		void TryConnect();

		EditorTextField* Url = nullptr;
		EditorTextField* Password = nullptr;

		std::function<void(ConnectResult)> SubmitResult;
	};
}