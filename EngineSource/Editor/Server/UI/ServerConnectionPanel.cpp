#include "ServerConnectionPanel.h"
#include <Editor/UI/EditorUI.h>
#include <Core/Error/EngineAssert.h>
#include <Engine/MainThread.h>

using namespace kui;

engine::editor::ServerConnectionPanel::ServerConnectionPanel(ServerConnection* Connection)
	: EditorPanel("Editor Server", "connection")
{
	Element = new ServerConnectionPanelElement();
	this->Connection = Connection;
	this->Background->AddChild(Element);

	Connection->OnUsersChanged.Add(this, [this] {
		UpdateUsers();
	});

	Connection->OnClosed.Add(this, [this] {
		this->Connection = nullptr;
		delete this;
	});

	Connection->OnChatMessage.Add(this, [this] {
		UpdateChatMessages();
	});

	Element->chatField->field->OnChanged = [this]
	{
		string Message = Element->chatField->field->GetText();

		if (!Message.empty())
		{
			this->Connection->SendMessage("sendChat", Message);

			Element->chatField->field->SetText("");
		}
	};
}

engine::editor::ServerConnectionPanel::~ServerConnectionPanel()
{
	if (Connection)
	{
		Connection->OnChatMessage.Remove(this);
		Connection->OnUsersChanged.Remove(this);
		Connection->OnClosed.Remove(this);
	}
}

void engine::editor::ServerConnectionPanel::Update()
{
}

void engine::editor::ServerConnectionPanel::OnResized()
{
	Element->SetWidth(this->Size.X - (201_px).GetScreen().X);
	Element->SetHeight(this->Size.Y - (34_px).GetScreen().Y);
}

void engine::editor::ServerConnectionPanel::OnThemeChanged()
{
	UpdateUsers();
	UpdateChatMessages();
}

void engine::editor::ServerConnectionPanel::UpdateUsers()
{
	this->Element->users->DeleteChildren();
	for (auto& i : Connection->Users)
	{
		this->Element->users->AddChild((new UIText(11_px, EditorUI::Theme.Text, i + (Connection->ThisUserName == i ? " (You)" : ""), EditorUI::EditorFont))
			->SetPadding(0, 5_px, 0, 0));
	}
}

void engine::editor::ServerConnectionPanel::UpdateChatMessages()
{
	this->Element->chat->DeleteChildren();
	for (auto& i : Connection->Chat)
	{
		this->Element->chat->AddChild((new UIText(11_px, EditorUI::Theme.Text, i.Sender + ": " + i.Message, EditorUI::EditorFont))
			->SetPadding(5_px, 5_px, 5_px, 0));
	}
}
