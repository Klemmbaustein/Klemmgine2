#ifdef EDITOR
#include "VectorField.h"
#include <kui/UI/UIBackground.h>
#include <Engine/Editor/UI/EditorUI.h>
#include <stdexcept>
using namespace kui;

static std::array<Vec3f, 3> Colors = {
	Vec3f(0.6f, 0.1f, 0.0f),
	Vec3f(0.0f, 0.6f, 0.1f),
	Vec3f(0.0f, 0.1f, 0.6f),
};

static engine::string FloatToString(float Val)
{
	engine::string Out = std::to_string(Val);

	while (Out.size()
		&& (Out[Out.size() - 1] == '0' || Out[Out.size() - 1] == '.'))
	{
		if (Out[Out.size() - 1] == '.')
		{
			Out.pop_back();
			break;
		}
		Out.pop_back();
	}
	return Out;
}

engine::editor::VectorField::VectorField(Vector3 InitialValue, float Size, std::function<void()> OnChanged)
	: kui::UIBox(true, 0)
{
	this->OnChanged = OnChanged;
	this->Value = InitialValue;
	this->TextFields = {
		nullptr, nullptr, nullptr
	};

	float LabelSize = PixelSizeToScreenSize(12, ParentWindow).X;
	float ElementSize = (Size - LabelSize * 3) / 3.0f;

	for (int32 i = 0; i < TextFields.size(); i++)
	{
		auto* CoordBackground = new UIBackground(0, 1, Colors[i], kui::Vec2f(LabelSize, 0));
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
					Value[i] = stoi(TextFields[i]->GetText());
				}
				catch (std::invalid_argument)
				{
					TextFields[i]->SetText(FloatToString(Value[i]));
				}
				catch (std::out_of_range)
				{
					TextFields[i]->SetText(FloatToString(Value[i]));
				}
				this->OnChanged();
			});
		AddChild(NewField
			->SetText(FloatToString(Value[i]))
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
		TextFields[i]->SetText(FloatToString(Value[i]));
	}
}
#endif