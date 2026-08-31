#include "LandscapeObjectEditor.h"
#include <Engine/Objects/LandscapeObject.h>
#include <kui/UI/UIBackground.h>
#include <Editor/UI/EditorUI.h>
#include <Common.kui.hpp>
#include <Core/Log.h>
#include <Editor/UI/Elements/PropertyMenu.h>
#include <Editor/UI/Panels/Viewport.h>
#include <Engine/Stats.h>
#include <Engine/Input.h>

using namespace kui;

kui::UIBox* engine::editor::LandscapeObjectEditor::ShowContextUI(Viewport* ToView, SceneObject* Object)
{
	UIBox* ResultBox = new UIBox(false);
	ResultBox->SetMinWidth(UISize::Parent(1));

	Mode = LandscapeEditMode::Select;

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

	this->Edited = static_cast<LandscapeObject*>(Object);

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

	PropertyMenu* Properties = new PropertyMenu();
	Properties->SetMinWidth(150_px);
	Properties->SetMaxWidth(150_px);
	ResultBox->AddChild(Properties);

	Properties->AddFloatEntry("Brush size", BrushSize, nullptr);
	Properties->AddFloatEntry("Intensity", Intensity, nullptr);

	return ResultBox;
}

bool engine::editor::LandscapeObjectEditor::HandleMouseClick(Viewport* View, bool Held)
{
	if (Mode == LandscapeEditMode::Select)
	{
		return false;
	}

	auto hit = View->RayAtCursor(1000, 1000);

	if (!hit.Hit || hit.HitComponent->GetRootObject() != Edited)
	{
		return true;
	}

	View->SceneChanged();

	float Direction = input::IsKeyHeld(input::Key::SHIFT) ? -1.0f : 1.0f;

	Vector3 RelativeCoordinates = Edited->Component->WorldTransform.Inverse().ApplyTo(hit.ImpactPoint);

	int64 x = std::round(RelativeCoordinates.X);
	int64 y = std::round(RelativeCoordinates.Z);

	std::lock_guard g{ Edited->Component->Data->Lock };

	int64 IntRange = std::ceil(this->BrushSize);

	for (int64 ix = x - IntRange; ix < x + IntRange; ix++)
	{
		for (int64 iy = y - IntRange; iy < y + IntRange; iy++)
		{
			if (ix < 0 || iy < 0)
			{
				continue;
			}

			auto& p = Edited->Component->Data->GetPointAt(ix, iy);

			float DistanceIntensity = std::max(this->BrushSize - Vector3::Distance(Vector3(x, y, 0), Vector3(ix, iy, 0)), 0.0f) / BrushSize;

			DistanceIntensity = DistanceIntensity * DistanceIntensity * (3.0f - 2.0 * DistanceIntensity);

			p.Height += stats::DeltaTime * DistanceIntensity * Intensity * Direction;
		}
	}

	Edited->Component->Data->InitializeNormalForRange(
		RelativeCoordinates.X - IntRange - 1, RelativeCoordinates.Z - IntRange - 1,
		IntRange * 2 + 2, IntRange * 2 + 2);

	Edited->Component->IsDirty = true;
	Edited->Component->Data->Changed = true;

	return true;
}

engine::editor::LandscapeObjectEditor::LandscapeObjectEditor()
{
	this->Type = LandscapeObject::ObjectType;
}
