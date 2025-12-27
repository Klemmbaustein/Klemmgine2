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
	this->Value = not this->Value;
	SetUseTexture(this->Value, EditorUI::Asset("Checkbox.png"));
	UIButton::OnButtonClicked();
}