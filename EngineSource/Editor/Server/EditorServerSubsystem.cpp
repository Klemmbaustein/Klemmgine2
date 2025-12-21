#include "EditorServerSubsystem.h"
#include <Core/File/JsonSerializer.h>
#include <Engine/Engine.h>
#include <Engine/Script/ScriptSubsystem.h>
#include <Editor/UI/EditorUI.h>
#include <Editor/Server/UI/ServerConnectionPanel.h>
#include <Editor/UI/Panels/MessagePanel.h>
#include <Editor/Server/ServerAssetsProvider.h>
#include <Editor/Server/ServerScriptsProvider.h>
#include <Editor/Server/ServerResourceSource.h>
#include <sstream>
using namespace engine::editor;

EditorServerSubsystem::EditorServerSubsystem(ServerConnection* Connection)
	: subsystem::Subsystem("Editor Server", Log::LogColor::Yellow)
{
	this->Connection = Connection;
	this->Connection->SendMessage("initialize", "");

	auto UI = EditorUI::Instance;

	UI->ForEachPanel<MessagePanel>([this](MessagePanel* p) {
		p->AddChild(new ServerConnectionPanel(this->Connection), EditorPanel::Align::Tabs, true, 0);
	});

	UI->Update();

	Engine::GetSubsystem<script::ScriptSubsystem>()->Scripts = new ServerScriptProvider(Connection);
	Engine::GetSubsystem<script::ScriptSubsystem>()->Reload();

	Event<> OldEvent = UI->AssetsProvider->OnChanged;
	UI->AssetsProvider = new ServerAssetsProvider(Connection);
	UI->AssetsProvider->OnChanged = OldEvent;
	resource::AddResourceSource(new ServerResourceSource(Connection));
}

engine::editor::EditorServerSubsystem::~EditorServerSubsystem()
{
	delete Connection;
}

