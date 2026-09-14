#include "EditorServerSubsystem.h"
#include <Engine/Engine.h>
#include <Engine/Script/ScriptSubsystem.h>
#include <Editor/UI/EditorUI.h>
#include <Editor/Server/UI/ServerConnectionPanel.h>
#include <Editor/UI/Panels/ConsolePanel.h>
#include <Editor/Server/ServerAssetsProvider.h>
#include <Editor/Server/ServerResourceSource.h>
#include <Core/Networking/HttpWebSocket.h>
using namespace engine::editor;

EditorServerSubsystem::EditorServerSubsystem(ServerConnection* Connection)
	: subsystem::Subsystem("Editor Server", Log::LogColor::Yellow)
{
	this->Connection = Connection;
	this->Connection->SendMessage("initialize", SerializedValue());

	auto UI = EditorUI::Instance;

	if (UI)
	{
		OnEditorLoaded(UI);

		UI->Update();

		Event<> OldEvent = UI->AssetsProvider->OnChanged;
		UI->LoadAssetProvider(new ServerAssetsProvider(Connection));
		UI->AssetsProvider->OnChanged = OldEvent;
	}

	resource::AddResourceSource(new ServerResourceSource(Connection));
	Engine::GetSubsystem<script::ScriptSubsystem>()->Reload();
}

engine::editor::EditorServerSubsystem::~EditorServerSubsystem()
{
	delete Connection;
}

void engine::editor::EditorServerSubsystem::OnEditorLoaded(EditorUI* UI)
{
	ConsolePanel* c = nullptr;

	UI->ForEachPanel<ConsolePanel>([&c](ConsolePanel* p) {
		c = p;
	});

	if (c)
	{
		c->AddChild(new ServerConnectionPanel(this->Connection), EditorPanel::Align::Tabs, true, 0);
	}
}
