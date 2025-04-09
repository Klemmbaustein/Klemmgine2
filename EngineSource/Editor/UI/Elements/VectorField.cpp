#ifdef EDITOR
#include "VectorField.h"
#include <kui/UI/UIBackground.h>
#include <Editor/UI/EditorUI.h>
#include <stdexcept>
using namespace kui;

static std::array<Vec3f, 3> Colors = {
	Vec3f(0.6f, 0.1f, 0.0f),
	Vec3f(0.0f, 0.6f, 0.1f),
	Vec3f(0.0f, 0.1f, 0.6f),
};

engine::editor::VectorField::VectorField(Vector3 InitialValue, UISize Size, std::function<void()> OnChanged)
	: kui::UIBox(true, 0)
{
	this->OnChanged = OnChanged;
	this->Value = InitialValue;
	this->TextFields = {
		nullptr, nullptr, nullptr
	};

	float LabelSize = (12_px).GetScreen().X;
	float ElementSize = (Size.GetScreen().X - LabelSize * 3) / 3.0f;

	for (int32 i = 0; i < TextFields.size(); i++)
	{
		auto* CoordBackground = new UIBackground(0, 1, Colors[i], SizeVec(LabelSize, 0));
		CoordBackground
			->SetMinHeight(UISize::Parent(1))
			->SetVerticalAlign(UIBox::Align::Centered)
			->SetHorizontalAlign(UIBox::Align::Centered)
			->AddChild((new UIText(UISize::Pixels(11), 1, string({ char('X' + i) }), EditorUI::EditorFont)));

		AddChild(CoordBackground);

		auto* NewField = new UITextField(0, EditorUI::Theme.DarkBackground, EditorUI::EditorFont, [this, i]()
			{
				try
				{
					Value[i] = std::stof(TextFields[i]->GetText());
				}
				catch (std::invalid_argument)
				{
					TextFields[i]->SetText(str::FloatToString(Value[i], 3));
				}
				catch (std::out_of_range)
				{
					TextFields[i]->SetText(str::FloatToString(Value[i], 3));
				}
				this->OnChanged();
			});
		AddChild(NewField
			->SetText(str::FloatToString(Value[i], 3))
			->SetTextColor(EditorUI::Theme.Text)
			->SetTextSize(UISize::Pixels(11))
			->SetMinSize(Vec2f(ElementSize, 0)));
		TextFields[i] = NewField;
	}
	UpdateElement();
}

engine::editor::VectorField::~VectorField()
{
}

engine::Vector3 engine::editor::VectorField::GetValue() const
{
	return Value;
}

void engine::editor::VectorField::SetValue(Vector3 NewValue)
{
	if (Value != NewValue)
	{
		Value = NewValue;
		UpdateValues();
	}
}

void engine::editor::VectorField::UpdateValues()
{
	for (size_t i = 0; i < TextFields.size(); i++)
	{
		if (!TextFields[i]->GetIsEdited())
		{
			TextFields[i]->SetText(str::FloatToString(Value[i], 3));
		}
	}
}
#endif