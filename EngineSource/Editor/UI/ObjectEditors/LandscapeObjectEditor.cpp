#include "LandscapeObjectEditor.h"
#include <Engine/Objects/LandscapeObject.h>
#include <kui/UI/UIBackground.h>
#include <Editor/UI/EditorUI.h>
#include <Common.kui.hpp>

using namespace kui;

kui::UIBox* engine::editor::LandscapeObjectEditor::ShowContextUI(Viewport* ToView, SceneObject* Object)
{
	UIBox* ResultBox = new UIBox(false);
	ResultBox->SetMinWidth(UISize::Parent(1));

	std::map<LandscapeEditMode, string> Modes = {
		{LandscapeEditMode::Select, "Select"},
		{LandscapeEditMode::Shape, "Shape"},
		{LandscapeEditMode::Flatten, "Flatten"},
	};

	auto SelectButton = [](UIButton* btn) {
		btn->SetColor(EditorUI::Theme.HighlightDark);
		btn->SetHoveredColor(EditorUI::Theme.HighlightDark);
		btn->SetPressedColor(EditorUI::Theme.HighlightDark);
		btn->SetBorder(1_px, EditorUI::Theme.Highlight1);
	};

	auto UnSelectButton = [](UIButton* btn) {
		btn->SetColor(EditorUI::Theme.Background);
		btn->SetHoveredColor(EditorUI::Theme.Background * 0.75f);
		btn->SetKeyboardHoveredColor(EditorUI::Theme.Background * 0.75f);
		btn->SetPressedColor(EditorUI::Theme.Background * 0.5f);
		btn->SetBorder(1_px, EditorUI::Theme.Highlight1);
	};

	ModeButtons.clear();
	for (auto& i : Modes)
	{
		UIButton* NewButton = new UIButton(true, 0, EditorUI::Theme.Background, nullptr);

		NewButton->OnClicked = [this, Modes, SelectButton, UnSelectButton, NewButton, NewMode = i.first] {
			this->Mode = NewMode;
			for (auto& i : ModeButtons)
			{
				if (i != NewButton)
				{
					UnSelectButton(i);
				}
			}
			SelectButton(NewButton);
		};

		if (i.first == this->Mode)
		{
			SelectButton(NewButton);
		}
		else
		{
			UnSelectButton(NewButton);
		}

		NewButton
			->SetCorner(EditorUI::Theme.CornerSize)
			->SetMinWidth(UISize::Parent(1))
			->SetPadding(5_px)
			->SetHorizontalAlign(UIBox::Align::Centered)
			->AddChild((new UIText(12_px, EditorUI::Theme.Text, i.second, EditorUI::EditorFont))
				->SetPadding(3_px));

		ModeButtons.push_back(NewButton);
		ResultBox->AddChild(NewButton);
	}

	return ResultBox;
}

engine::editor::LandscapeObjectEditor::LandscapeObjectEditor()
{
	this->Type = LandscapeObject::ObjectType;
}
