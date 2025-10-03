#include "EditorServerSubsystem.h"
#include <Core/File/JsonSerializer.h>
#include <Editor/UI/EditorUI.h>
#include <Editor/Server/ServerAssetsProvider.h>
#include <Editor/Server/ServerResourceSource.h>
#include <sstream>
using namespace engine::editor;

EditorServerSubsystem::EditorServerSubsystem(ServerConnection* Connection)
	: subsystem::Subsystem("Editor Server", Log::LogColor::Yellow)
{
	this->Connection = Connection;

	this->Connection->SendMessage("initialize", "");

	EditorUI::Instance->AssetsProvider = new ServerAssetsProvider(Connection);
	resource::AddResourceSource(new ServerResourceSource(Connection));
}

engine::editor::EditorServerSubsystem::~EditorServerSubsystem()
{
	delete Connection;
}

