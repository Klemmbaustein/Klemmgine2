#include "EditorServerSubsystem.h"
#include <Core/File/JsonSerializer.h>
#include <Editor/UI/EditorUI.h>
#include <Editor/Server/UI/ServerConnectionPanel.h>
#include <Editor/UI/Panels/MessagePanel.h>
#include <Editor/Server/ServerAssetsProvider.h>
#include <Editor/Server/ServerResourceSource.h>
#include <sstream>
using namespace engine::editor;

EditorServerSubsystem::EditorServerSubsystem(ServerConnection* Connection)
	: subsystem::Subsystem("Editor Server", Log::LogColor::Yellow)
{
	this->Connection = Connection;

	this->Connection->SendMessage("initialize", "");

	auto OldEvent = EditorUI::Instance->AssetsProvider->OnChanged;

	bool Found = false;

	EditorUI::Instance->ForEachPanel<MessagePanel>([&](MessagePanel* p) {
		Found = true;
		p->AddChild(new ServerConnectionPanel(this->Connection), EditorPanel::Align::Tabs, true);
	});

	EditorUI::Instance->Update();

	EditorUI::Instance->AssetsProvider = new ServerAssetsProvider(Connection);
	EditorUI::Instance->AssetsProvider->OnChanged = OldEvent;
	resource::AddResourceSource(new ServerResourceSource(Connection));
}

engine::editor::EditorServerSubsystem::~EditorServerSubsystem()
{
	delete Connection;
}

