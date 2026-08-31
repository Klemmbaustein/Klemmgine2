#include "Checkbox.h"
#include <Editor/UI/EditorUI.h>

using namespace kui;
using namespace engine::editor;

UICheckbox::UICheckbox(bool Value, std::function<void()> OnClicked)
	: UIButton(true, 0, 1, OnClicked)
{
	this->Value = Value;
	SetUseTexture(Value, EditorUI::Asset("Checkbox.png"));
	SetMinSize(16_px);
	SetBorder(1_px, EditorUI::Theme.BackgroundHighlight);
}

void UICheckbox::OnButtonClicked()
{
	this->Value = !this->Value;
	SetUseTexture(this->Value, EditorUI::Asset("Checkbox.png"));
	UIButton::OnButtonClicked();
}

void engine::editor::UICheckbox::UpdateImage()
{
	SetUseTexture(this->Value, EditorUI::Asset("Checkbox.png"));
}
