#pragma once
#include <Editor/UI/Panels/EditorPanel.h>
#include <Editor/Server/ServerConnection.h>
#include <ServerConnectionPanel.kui.hpp>

namespace engine::editor
{
	class ServerConnectionPanel : public EditorPanel
	{
	public:

		ServerConnectionPanel(ServerConnection* Connection);
		~ServerConnectionPanel();

		virtual void Update() override;
		virtual void OnResized() override;
		virtual void OnThemeChanged() override;

		ServerConnection* Connection = nullptr;
		ServerConnectionPanelElement* Element = nullptr;

		void UpdateUsers();
		void UpdateChatMessages();
	};
}