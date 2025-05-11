#ifdef EDITOR
#include "PropertyPanel.h"
#include <Editor/UI/EditorUI.h>
#include <kui/UI/UITextField.h>
#include "Viewport.h"
#include <kui/Window.h>
#include <Engine/Scene.h>
#include <PropertyPanel.kui.hpp>
#include <Editor/UI/Elements/VectorField.h>
#include <Editor/UI/Elements/Checkbox.h>
#include <Core/Log.h>
#include <Editor/UI/Elements/AssetSelector.h>
using namespace kui;

engine::editor::PropertyPanel::PropertyPanel()
	: EditorPanel("Properties", "object_properties")
{
	LoadPropertiesFrom(nullptr);
}

void engine::editor::PropertyPanel::Update()
{
	if (Viewport::Current && Scene::GetMain())
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
		else if (SelectedObj && OldObjectTransform.Matrix != SelectedObj->ObjectTransform.Matrix)
		{
			for (auto& UpdateFn : UpdateProperties)
			{
				UpdateFn();
			}
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
	UpdateProperties.clear();

	UIScrollBox* ContentBox = new UIScrollBox(false, 0, true);
	ContentBox->SetMinSize(UISize::Parent(1));
	ContentBox->SetMaxSize(UISize::Parent(1));
	Background->AddChild(ContentBox);

	if (!Object)
	{
		ContentBox->SetHorizontalAlign(UIBox::Align::Centered);
		ContentBox->AddChild((new UIText(11_px, EditorUI::Theme.Text, "Nothing selected", EditorUI::EditorFont))
			->SetPadding(10_px));
		return;
	}

	int32 PixelSize = int32(Size.X
		* float(Background->GetParentWindow()->GetSize().X)
		/ float(Background->GetParentWindow()->GetDPI())
		/ 2.0);

	bool Compact = PixelSize < 220;
	float Size = UIBox::PixelSizeToScreenSize(kui::Vec2f(float(PixelSize - (Compact ? 10 : 100))), Background->GetParentWindow()).X;

	auto CreateNewHeading = [this, ContentBox](string Title, bool HasPadding = true) -> PropertyHeaderElement*
		{
			auto* New = new PropertyHeaderElement();

			New->SetTitle(Title);
			ContentBox->AddChild(New);
			if (!HasPadding)
				New->SetUpPadding(5_px);
			else
				New->SetUpPadding(15_px);

			return New;
		};

	auto CreateNewEntry = [this, Compact, ContentBox](string Name) -> PropertyEntryElement*
		{
			auto* New = new PropertyEntryElement();
			New->SetPropertyName(Name);

			if (Compact)
			{
				New->SetHorizontal(false);
				New->SetVerticalAlign(UIBox::Align::Reverse);
				New->textBox->SetHorizontalAlign(UIBox::Align::Default);
				New->SetCompactTextPadding(0_px);
				delete New->separator;
			}

			ContentBox->AddChild(New);
			return New;
		};

	CreateNewHeading("Object: " + Object->Name + "\nClass: " + Reflection::ObjectTypes[Object->TypeID].Name + "", false);

	CreateNewHeading("Object");


	auto AddVecEntry = [this, Object, Size, CreateNewEntry](string Name, Vector3& Value)
		{
			auto* Position = CreateNewEntry(Name);

			auto* PosField = new VectorField(Value, Size, nullptr);
			PosField->OnChanged = [&Value, Object, PosField]()
				{
					Viewport::Current->OnObjectChanged(Object);
					Value = PosField->GetValue();
				};
			Position->valueBox->AddChild(PosField);
			UpdateProperties.push_back([PosField, &Value]()
				{
					PosField->SetValue(Value);
				});
		};

	AddVecEntry("Position", Object->Position);
	AddVecEntry("Rotation", *(Vector3*)&Object->Rotation);
	AddVecEntry("Scale", *(Vector3*)&Object->Scale);

	auto* Name = CreateNewEntry("Name");

	auto* NameField = new UITextField(0, EditorUI::Theme.DarkBackground, EditorUI::EditorFont, nullptr);

	NameField->OnChanged = [Object, NameField]()
		{
			Viewport::Current->OnObjectChanged(Object);
			Object->Name = NameField->GetText();
		};

	Name->valueBox->AddChild(NameField
		->SetText(Object->Name)
		->SetTextColor(EditorUI::Theme.Text)
		->SetTextSize(11_px)
		->SetMinSize(kui::Vec2f(Size, 0)));

	CreateNewHeading(Reflection::ObjectTypes[Object->TypeID].Name);

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

			Selector->OnChanged = [Object, Selector, Ref]()
				{
					if (Selector->SelectedAsset.Extension != Ref->Value.Extension)
						return;
					Viewport::Current->OnObjectChanged(Object);
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
			auto* TextField = new UITextField(0, EditorUI::Theme.DarkBackground, EditorUI::EditorFont, nullptr);

			TextField->OnChanged = [Object, TextField, Ref]()
			{
				Viewport::Current->OnObjectChanged(Object);
				Ref->Value = TextField->GetText();
				if (Ref->OnChanged)
					Ref->OnChanged();
			};

			New->valueBox->AddChild(TextField
				->SetText(Ref->Value)
				->SetTextSize(11_px)
				->SetMinSize(kui::Vec2f(Size, 0)));
			break;
		}
		case PropertyType::Bool:
		{
			auto* Ref = static_cast<ObjProperty<bool>*>(i);

			auto* Checkbox = new UICheckbox(Ref->Value, nullptr);
			Checkbox->OnClicked = [Object, Checkbox, Ref]()
			{
				Viewport::Current->OnObjectChanged(Object);
				Ref->Value = Checkbox->Value;
				if (Ref->OnChanged)
					Ref->OnChanged();

			};

			New->valueBox->AddChild(Checkbox);
			break;
		}
		default:
			break;
		}
	}
	Background->UpdateElement();
	Background->RedrawElement();
}
#endif