#include "PropertyPanel.h"
#include <Editor/UI/EditorUI.h>
#include <kui/UI/UITextField.h>
#include "Viewport.h"
#include <kui/Window.h>
#include <Engine/Scene.h>
#include <PropertyPanel.kui.hpp>
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
		else if (UpdateTimer.Get() > 0.1f && SelectedObj
			&& OldObjectTransform.Matrix != SelectedObj->ObjectTransform.Matrix)
		{
			this->UpdateTimer.Reset();
			OldObjectTransform = SelectedObj->ObjectTransform.Matrix;
			Properties->UpdateProperties();
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
	LoadPropertiesFrom(SelectedObj);
}

void engine::editor::PropertyPanel::LoadPropertiesFrom(SceneObject* Object)
{
	Background->DeleteChildren();

	Properties = new PropertyMenu();
	Properties->SetMinSize(UISize::Parent(1));
	Properties->SetMaxSize(UISize::Parent(1));
	Background->AddChild(Properties);
	Properties->Clear();

	if (!Object)
	{
		Properties->SetMode(PropertyMenu::Mode::DisplayText);
		Properties->AddChild((new UIText(11_px, EditorUI::Theme.Text, "Nothing selected", EditorUI::EditorFont))
			->SetPadding(10_px));
		return;
	}

	Properties->SetMode(PropertyMenu::Mode::DisplayEntries);
	Properties->CreateNewHeading("Object: " + Object->Name, EditorUI::Instance->ObjectIcons.GetObjectIcon(Object->TypeID));

	Properties->CreateNewHeading("Object");

	auto OnObjectChanged = [Obj = SelectedObj] {
		Viewport::Current->OnObjectChanged(Obj);
	};

	Properties->AddVecEntry("Position", Object->Position, OnObjectChanged);
	Properties->AddVecEntry("Rotation", *(Vector3*)&Object->Rotation, OnObjectChanged, false, true);
	Properties->AddVecEntry("Scale", Object->Scale, OnObjectChanged);

	Properties->AddStringEntry("Name", Object->Name, OnObjectChanged);

	Properties->AddInfoEntry("Class", Reflection::ObjectTypes[Object->TypeID].Name);

	Properties->CreateNewHeading(Reflection::ObjectTypes[Object->TypeID].Name);
	OldObjectTransform = SelectedObj->ObjectTransform.Matrix;

	for (ObjPropertyBase* i : Object->Properties)
	{
		if (i->IsHidden)
			continue;
		if (i->Type == PropertyType::Unknown)
			continue;

		switch (i->Type)
		{
		case PropertyType::AssetRef:
		{
			auto* Ref = static_cast<ObjProperty<AssetRef>*>(i);
			Properties->AddAssetRefEntry(i->Name, Ref->Value, [Object, Ref] {
				Viewport::Current->OnObjectChanged(Object);
				if (Ref->OnChanged)
					Ref->OnChanged();
			});

			break;
		}
		case PropertyType::String:
		{
			auto* Ref = static_cast<ObjProperty<string>*>(i);
			Properties->AddStringEntry(i->Name, Ref->Value, [Object, Ref] {
				Viewport::Current->OnObjectChanged(Object);
				if (Ref->OnChanged)
					Ref->OnChanged();
			});
			break;
		}
		case PropertyType::Int:
		{
			auto* Ref = static_cast<ObjProperty<int32>*>(i);
			Properties->AddIntEntry(i->Name, Ref->Value, [Object, Ref] {
				Viewport::Current->OnObjectChanged(Object);
				if (Ref->OnChanged)
					Ref->OnChanged();
			});
			break;
		}
		case PropertyType::Float:
		{
			auto* Ref = static_cast<ObjProperty<float>*>(i);
			Properties->AddFloatEntry(i->Name, Ref->Value, [Object, Ref] {
				Viewport::Current->OnObjectChanged(Object);
				if (Ref->OnChanged)
					Ref->OnChanged();
			});
			break;
		}
		case PropertyType::Bool:
		{
			auto* Ref = static_cast<ObjProperty<bool>*>(i);
			Properties->AddBooleanEntry(Ref->Name, Ref->Value, [Object, Ref]() {
				Viewport::Current->OnObjectChanged(Object);
				if (Ref->OnChanged)
					Ref->OnChanged();

			});
			break;
		}
		case PropertyType::Vector3:
		{
			auto* Ref = static_cast<ObjProperty<Vector3>*>(i);
			Properties->AddVecEntry(Ref->Name, Ref->Value, [Object, Ref]() {
				Viewport::Current->OnObjectChanged(Object);
				if (Ref->OnChanged)
					Ref->OnChanged();

			}, Ref->HasHint(PropertyHint::Vec3Color));
			break;
		}

		default:
			break;
		}
	}
}

void engine::editor::PropertyPanel::OnThemeChanged()
{
	OnResized();
}
