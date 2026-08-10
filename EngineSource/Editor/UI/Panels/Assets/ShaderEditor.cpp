#include "ShaderEditor.h"
#include <Editor/UI/EditorUI.h>
#include <Editor/UI/Elements/Toolbar.h>

using namespace kui;

engine::editor::ShaderEditor::ShaderEditor(AssetRef ShaderFile)
	: AssetEditor("Shader: %s", ShaderFile)
{
	Toolbar* EditorToolbar = new Toolbar();
	EditorToolbar->AddButton("Save", EditorUI::Asset("Save.png"), [this]() {
		Save();
	});
	Background->SetHorizontal(false);

	Background->AddChild(EditorToolbar);

	Provider = new ShaderEditorProvider(ShaderFile.FilePath,
		EditorUI::Theme.CodeTheme.Variable, EditorUI::Theme.CodeTheme.Function);

	EditorUI::Theme.CodeTheme.ApplyToFile(Provider);

	Editor = new UITextEditor(Provider, EditorUI::Instance->MonospaceFont);
	Background->AddChild(Editor
		->SetPadding(5_px));

	Editor->SetMinSize(UISize::Parent(1));
	Editor->SetMaxSize(UISize::Parent(1));

	auto CtrlMod = ShortcutModifiers{ .Ctrl = true };

	AddShortcut(Key::z, CtrlMod, [this] {
		Provider->Undo();
	}, ShortcutOptions::AllowInText);

	AddShortcut(Key::y, CtrlMod, [this] {
		Provider->Redo();
	}, ShortcutOptions::AllowInText);

	AddShortcut(Key::s, CtrlMod, [this] {
		Save();
	}, ShortcutOptions::AllowInText);

	EditorToolbar->AddButton("Undo", EditorUI::Asset("Undo.png"), [this]() {
		Provider->Undo();
	});

	EditorToolbar->AddButton("Redo", EditorUI::Asset("Redo.png"), [this]() {
		Provider->Redo();
	});
}

void engine::editor::ShaderEditor::Update()
{
	if (Provider->IsChanged)
	{
		this->OnChanged();
	}
}

void engine::editor::ShaderEditor::Save()
{
	AssetEditor::Save();
	std::ofstream out = std::ofstream(this->EditedAsset.FilePath);

	out << this->Provider->GetContent();
	Provider->IsChanged = false;
	out.close();
	EditorUI::Instance->OnProjectAssetChanged(this->EditedAsset);
	Provider->Reload();
	Editor->FullRefresh();
}

void engine::editor::ShaderEditor::OnThemeChanged()
{
	EditorUI::Theme.CodeTheme.ApplyToFile(Provider);
	Provider->VariableColor = EditorUI::Theme.CodeTheme.Variable;
	Provider->FunctionColor = EditorUI::Theme.CodeTheme.Function;
	Editor->SelectionColor = EditorUI::Theme.SelectedText;
	Editor->CursorColor = EditorUI::Theme.Text;
	Provider->Reload();
	Editor->FullRefresh();
}

void engine::editor::ShaderEditor::OnResized()
{
	this->Editor->SetMinHeight(this->Size.Y - (78_px).GetScreen().Y);
	this->Editor->SetMinWidth(UISize::Parent(1));
}
