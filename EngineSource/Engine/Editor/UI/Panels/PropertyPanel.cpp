#include "PropertyPanel.h"
#include <Engine/Editor/UI/EditorUI.h>
#include <kui/UI/UITextField.h>
#include "Viewport.h"
#include <kui/Window.h>
#include <Engine/Scene.h>
#include <PropertyPanel.kui.hpp>
#include <Engine/Editor/UI/Elements/VectorField.h>
#include <Engine/Editor/UI/Elements/AssetSelector.h>
using namespace kui;

engine::editor::PropertyPanel::PropertyPanel()
	: EditorPanel("Properties", "object_properties")
{
}

void engine::editor::PropertyPanel::Update()
{
	if (Viewport::Current)
	{
		SceneObject* New = nullptr;
		if (!Viewport::Current->SelectedObjects.empty())
		{
			New = *Viewport::Current->SelectedObjects.begin();
		}

		if (New != SelectedObj)
		{
			SelectedObj = New;
			LoadPropertiesFrom(SelectedObj);
		}
	}
	else
	{
		if (SelectedObj)
		{
			SelectedObj = nullptr;
			LoadPropertiesFrom(SelectedObj);
		}
	}
}

void engine::editor::PropertyPanel::OnResized()
{
	if (SelectedObj)
	{
		LoadPropertiesFrom(SelectedObj);
	}
}

void engine::editor::PropertyPanel::LoadPropertiesFrom(SceneObject* Object)
{
	Background->DeleteChildren();

	if (!Object)
		return;

	int32 PixelSize = int32(Background->GetUsedSize().X
		* float(Background->GetParentWindow()->GetSize().X)
		/ float(Background->GetParentWindow()->GetDPI())
		/ 2.0);

	bool Compact = PixelSize < 220;
	float Size = UIBox::PixelSizeToScreenSize(kui::Vec2f(float(PixelSize - (Compact ? 10 : 90))), Background->GetParentWindow()).X;

	auto CreateNewHeading = [this](string Title) -> PropertyHeaderElement*
	{
		auto* New = new PropertyHeaderElement();

		New->SetTitle(Title);
		Background->AddChild(New);

		return New;
	};

	auto CreateNewEntry = [this, Compact](string Name) -> PropertyEntryElement*
	{
		auto* New = new PropertyEntryElement();
		New->SetPropertyName(Name);

		if (Compact)
		{
			New->SetHorizontal(false);
			New->SetVerticalAlign(UIBox::Align::Reverse);
			New->textBox->SetHorizontalAlign(UIBox::Align::Default);
			New->SetCompactTextPadding(0);
			delete New->separator;
		}

		Background->AddChild(New);
		return New;
	};

	CreateNewHeading("Object: " + Object->Name + "\nClass: " + Reflection::ObjectTypes[Object->TypeID].Name + "");

	auto* Position = CreateNewEntry("Position");

	auto* PosField = new VectorField(Object->Position, Size, nullptr);
	PosField->OnChanged = [Object, PosField]()
	{
		Object->Position = PosField->GetValue();
	};
	Position->valueBox->AddChild(PosField);

	for (ObjPropertyBase* i : Object->Properties)
	{
		if (i->Type == PropertyType::Unknown)
			continue;

		auto* New = CreateNewEntry(i->Name);

		switch (i->Type)
		{
		case PropertyType::AssetRef:
		{
			auto* Ref = static_cast<ObjProperty<AssetRef>*>(i);
			auto* Selector = new AssetSelector(Ref->Value, Size, nullptr);

			Selector->OnChanged = [Selector, Ref]()
			{
				Ref->Value = Selector->SelectedAsset;
				if (Ref->OnChanged)
					Ref->OnChanged();
			};

			New->valueBox->AddChild(Selector);
			break;
		}
		case PropertyType::String:
		{
			auto* Ref = static_cast<ObjProperty<string>*>(i);
			New->valueBox->AddChild((new UITextField(0, EditorUI::Theme.DarkBackground, EditorUI::EditorFont, nullptr))
				->SetText(Ref->Value)
				->SetTextSize(11)
				->SetTextSizeMode(UIBox::SizeMode::PixelRelative)
				->SetMinSize(kui::Vec2f(Size, 0)));
			break;
		}
		default:
			break;
		}
	}
	Background->RedrawElement();
}
