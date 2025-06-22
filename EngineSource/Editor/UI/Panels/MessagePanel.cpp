#include "MessagePanel.h"
#include <Editor/UI/EditorUI.h>
#include <kui/UI/UITextEditor.h>
#include <kui/UI/FileEditorProvider.h>
using namespace kui;

engine::editor::MessagePanel::MessagePanel()
	: EditorPanel("Messages", "messages")
{
	auto Provider = new FileEditorProvider("EngineSource/Core/File/TextSerializer.cpp");

	this->Background->AddChild((new UITextEditor(Provider, EditorUI::MonospaceFont))
		->SetMinSize(UISize::Parent(1))
		->SetPadding(5_px));
}

void engine::editor::MessagePanel::Update()
{
}
