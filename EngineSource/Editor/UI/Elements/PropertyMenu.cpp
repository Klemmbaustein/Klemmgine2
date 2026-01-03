#include "PropertyMenu.h"
#include "VectorField.h"
#include "Checkbox.h"
#include "AssetSelector.h"
#include <Editor/UI/EditorUI.h>
#include <Common.kui.hpp>

using namespace kui;

engine::editor::PropertyMenu::PropertyMenu(kui::Font* MenuFont)
	: UIScrollBox(false, 0, true)
{
	this->MenuFont = MenuFont;
}

void engine::editor::PropertyMenu::OnAttached()
{
	GetAbsoluteParent()->UpdateElement();
	Update();
}

void engine::editor::PropertyMenu::Clear()
{
	UpdatePropertiesCallback.clear();
	DeleteChildren();
	GetAbsoluteParent()->UpdateElement();
	Update();
}

PropertyHeaderElement* engine::editor::PropertyMenu::CreateNewHeading(string Title, std::string Icon, bool HasPadding)
{
	auto* New = new PropertyHeaderElement();

	New->SetTitle(Title);
	AddChild(New);

	New->SetUpPadding(HasPadding ? 10_px : 5_px);

	if (Icon.empty())
	{
		New->icon->IsCollapsed = true;
	}
	else
	{
		New->SetIcon(Icon);
	}

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
		New->separator->IsCollapsed = true;
	}

	AddChild(New);
	return New;
}

void engine::editor::PropertyMenu::AddVecEntry(string Name, Vector3& Value,
	std::function<void()> OnChanged, bool IsColor)
{
	auto* Position = CreateNewEntry(Name);

	auto* PosField = new VectorField(Value, ElementSize, nullptr, IsColor);
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

	auto* NameField = new UITextField(0, EditorUI::Theme.DarkBackground, this->MenuFont, nullptr);

	NameField->OnChanged = [OnChanged, &Value, NameField] {
		Value = NameField->GetText();
		if (OnChanged)
			OnChanged();
	};

	NameEntry->valueBox->AddChild(NameField
		->SetText(Value)
		->SetTextColor(EditorUI::Theme.Text)
		->SetInnerPadding(5_px)
		->SetTextSize(11_px)
		->SetBorder(1_px, EditorUI::Theme.DarkBackgroundHighlight)
		->SetCorner(EditorUI::Theme.CornerSize)
		->SetMinWidth(ElementSize)
		->SetMaxWidth(ElementSize));
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

void engine::editor::PropertyMenu::AddButtonEntry(string Name, string Label, std::function<void()> OnClicked)
{
	auto* NameEntry = CreateNewEntry(Name);

	auto* Button = new EditorButton();
	Button->btn->OnClicked = OnClicked;
	Button->btn->SetPadding(0);
	Button->SetText(Label);
	NameEntry->AddChild(Button);
}

void engine::editor::PropertyMenu::AddButtonsEntry(string Name, std::vector<std::pair<string, std::function<void()>>> Buttons)
{
	auto* NameEntry = CreateNewEntry(Name);

	for (auto& i : Buttons)
	{
		auto* Button = new EditorButton();
		Button->btn->OnClicked = i.second;
		Button->btn->SetPadding(0, 0, 0, 10_px);
		Button->SetText(i.first);
		NameEntry->AddChild(Button);
	}
}

void engine::editor::PropertyMenu::AddIntEntry(string Name, int32& Value, std::function<void()> OnChanged)
{
	auto* NameEntry = CreateNewEntry(Name);

	auto* NameField = new UITextField(0, EditorUI::Theme.DarkBackground, this->MenuFont, nullptr);

	NameField->OnChanged = [OnChanged, &Value, NameField] {
		try
		{
			Value = std::stoi(NameField->GetText());
		}
		catch (std::exception&)
		{
			NameField->SetText(std::to_string(Value));
		}
		if (OnChanged)
			OnChanged();
	};

	NameEntry->valueBox->AddChild(NameField
		->SetText(std::to_string(Value))
		->SetTextColor(EditorUI::Theme.Text)
		->SetInnerPadding(5_px)
		->SetTextSize(11_px)
		->SetBorder(1_px, EditorUI::Theme.DarkBackgroundHighlight)
		->SetCorner(EditorUI::Theme.CornerSize)
		->SetMinWidth(ElementSize)
		->SetMaxWidth(ElementSize));
}

void engine::editor::PropertyMenu::AddFloatEntry(string Name, float& Value, std::function<void()> OnChanged)
{
	auto* NameEntry = CreateNewEntry(Name);

	auto* NameField = new UITextField(0, EditorUI::Theme.DarkBackground, this->MenuFont, nullptr);

	NameField->OnChanged = [OnChanged, &Value, NameField] {
		try
		{
			Value = std::stof(NameField->GetText());
		}
		catch (std::exception&)
		{
			NameField->SetText(str::FloatToString(Value));
		}
		if (OnChanged)
			OnChanged();
	};

	NameEntry->valueBox->AddChild(NameField
		->SetText(str::FloatToString(Value))
		->SetTextColor(EditorUI::Theme.Text)
		->SetInnerPadding(5_px)
		->SetTextSize(11_px)
		->SetBorder(1_px, EditorUI::Theme.DarkBackgroundHighlight)
		->SetCorner(EditorUI::Theme.CornerSize)
		->SetMinWidth(ElementSize)
		->SetMaxWidth(ElementSize));
}
void engine::editor::PropertyMenu::SetMode(Mode NewMode)
{
	this->HorizontalBoxAlign = NewMode == Mode::DisplayText ? UIBox::Align::Centered : UIBox::Align::Default;
}

void engine::editor::PropertyMenu::AddAssetRefEntry(string Name, AssetRef& Value, std::function<void()> OnChanged,
	bool EmptyIsDefault)
{
	auto* New = CreateNewEntry(Name);
	auto* Selector = new AssetSelector(Value, ElementSize, nullptr, EmptyIsDefault);

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

void engine::editor::PropertyMenu::AddDropdownEntry(string Name,
	std::vector<kui::UIDropdown::Option> Values,
	std::function<void(kui::UIDropdown::Option)> OnChanged, size_t DefaultIndex)
{
	auto* New = CreateNewEntry(Name);
	auto* Dropdown = new UIDropdown(0, ElementSize, EditorUI::Theme.DarkBackground,
		EditorUI::Theme.Text, Values,
		[Values, OnChanged](int i) {
		OnChanged(Values[i]);
	}, this->MenuFont);

	Dropdown->SetTextSize(11_px, 3_px);
	Dropdown->SetDropdownColor(EditorUI::Theme.LightBackground, EditorUI::Theme.Text);
	Dropdown->SetBorder(1_px, EditorUI::Theme.DarkBackgroundHighlight);

	Dropdown->SelectOption(DefaultIndex, false);

	New->valueBox->AddChild(Dropdown);
}

void engine::editor::PropertyMenu::AddInfoEntry(string Name, string Value)
{
	auto* New = CreateNewEntry(Name);

	New->valueBox->AddChild((new UITextField(0, 0, this->MenuFont, nullptr))
		->SetCanEdit(false)
		->SetText(Value)
		->SetTextSize(11_px)
		->SetTextColor(EditorUI::Theme.Text)
		->SetOpacity(0)
		->SetMinSize(SizeVec(ElementSize, 15_px))
		->SetMaxSize(SizeVec(ElementSize, UISize::Largest())));
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
	ElementSize = UISize::Pixels(PixelSize - (IsCompact ? 10 : 110));
}
