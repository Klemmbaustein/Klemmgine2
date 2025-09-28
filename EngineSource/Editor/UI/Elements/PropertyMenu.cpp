#include "PropertyMenu.h"
#include "VectorField.h"
#include "Checkbox.h"
#include "AssetSelector.h"
#include <Editor/UI/EditorUI.h>

using namespace kui;

engine::editor::PropertyMenu::PropertyMenu()
	: UIScrollBox(false, 0, true)
{
}

void engine::editor::PropertyMenu::Clear()
{
	UpdatePropertiesCallback.clear();
	DeleteChildren();
	GetAbsoluteParent()->UpdateElement();
	Update();
}

PropertyHeaderElement* engine::editor::PropertyMenu::CreateNewHeading(string Title, bool HasPadding)
{
	auto* New = new PropertyHeaderElement();

	New->SetTitle(Title);
	AddChild(New);
	if (!HasPadding)
		New->SetUpPadding(5_px);
	else
		New->SetUpPadding(15_px);

	return New;
}

PropertyEntryElement* engine::editor::PropertyMenu::CreateNewEntry(string Name)
{
	auto* New = new PropertyEntryElement();
	New->SetPropertyName(Name);

	if (IsCompact)
	{
		New->SetHorizontal(false);
		New->SetVerticalAlign(UIBox::Align::Reverse);
		New->textBox->SetHorizontalAlign(UIBox::Align::Default);
		New->SetCompactTextPadding(0_px);
		delete New->separator;
	}

	AddChild(New);
	return New;
}

void engine::editor::PropertyMenu::AddVecEntry(string Name, Vector3& Value, std::function<void()> OnChanged)
{
	auto* Position = CreateNewEntry(Name);

	auto* PosField = new VectorField(Value, ElementSize, nullptr);
	PosField->OnChanged = [&Value, OnChanged, PosField] {
		Value = PosField->GetValue();
		if (OnChanged)
			OnChanged();
	};

	Position->valueBox->AddChild(PosField);
	UpdatePropertiesCallback.push_back([PosField, &Value] {
		PosField->SetValue(Value);
	});
}

void engine::editor::PropertyMenu::AddStringEntry(string Name, string& Value, std::function<void()> OnChanged)
{
	auto* NameEntry = CreateNewEntry(Name);

	auto* NameField = new UITextField(0, EditorUI::Theme.DarkBackground, EditorUI::EditorFont, nullptr);

	NameField->OnChanged = [OnChanged, &Value, NameField] {
		Value = NameField->GetText();
		if (OnChanged)
			OnChanged();
	};

	NameEntry->valueBox->AddChild(NameField
		->SetText(Value)
		->SetTextColor(EditorUI::Theme.Text)
		->SetTextSize(11_px)
		->SetMinSize(SizeVec(ElementSize, 0)));
}

void engine::editor::PropertyMenu::AddBooleanEntry(string Name, bool& Value, std::function<void()> OnChanged)
{
	auto* New = CreateNewEntry(Name);
	auto* Checkbox = new UICheckbox(Value, nullptr);
	Checkbox->OnClicked = [Checkbox, &Value, OnChanged]()
	{
		Value = Checkbox->Value;
		if (OnChanged)
			OnChanged();
	};

	New->valueBox->AddChild(Checkbox);
}

void engine::editor::PropertyMenu::AddAssetRefEntry(string Name, AssetRef& Value, std::function<void()> OnChanged)
{
	auto* New = CreateNewEntry(Name);
	auto* Selector = new AssetSelector(Value, ElementSize, nullptr);

	Selector->OnChanged = [Selector, &Value, OnChanged] {
		if (!Value.Extension.empty()
			&& Selector->SelectedAsset.Extension != Value.Extension)
			return;
		Value = Selector->SelectedAsset;
		if (OnChanged)
			OnChanged();
	};

	New->valueBox->AddChild(Selector);
}

void engine::editor::PropertyMenu::UpdateProperties()
{
	for (auto& i : UpdatePropertiesCallback)
	{
		i();
	}
}

void engine::editor::PropertyMenu::Update()
{
	UIScrollBox::Update();
	uint32 PixelSize = this->GetUsedSize().GetPixels().X;

	this->IsCompact = PixelSize < 220;
	ElementSize = UISize::Pixels(PixelSize - (IsCompact ? 10 : 100));
}
