#include "AssetSelector.h"
#include <kui/Window.h>
#include <Engine/File/Resource.h>
#include <Editor/UI/EditorUI.h>
#include <Core/File/FileUtil.h>
using namespace kui;

engine::editor::AssetSelector::AssetSelector(AssetRef InitialValue, kui::UISize Width,
	std::function<void()> OnChanged, bool EmptyIsDefault)
	: DroppableBox(true, [this](EditorUI::DraggedItem itm) {
	auto ref = AssetRef::FromPath(itm.Path);
	if (ref.Extension == this->SelectedAsset.Extension || this->SelectedAsset.Extension.empty())
	{
		SelectedAsset = ref;
		UpdateSelection();
		this->OnChanged();
	}
})
{
	IsAsset = true;
	this->OnChanged = OnChanged;
	this->EmptyIsDefault = EmptyIsDefault;
	SelectedAsset = InitialValue;
	auto c = EditorUI::GetExtIconAndColor(InitialValue.Extension);
	Init(c.first, c.second, Width);
}

engine::editor::AssetSelector::AssetSelector(ObjectTypeID InitialType, ObjectTypeID SuperType, kui::UISize Width,
	std::function<void()> OnChanged, bool EmptyIsDefault)
	: DroppableBox(true, [this](EditorUI::DraggedItem itm) {
	if (itm.ObjectType)
	{
		SelectedId = itm.ObjectType;
		UpdateObjectClass();
		UpdateSelection();
		this->OnChanged();
	}
})
{
	IsAsset = false;
	this->OnChanged = OnChanged;
	this->EmptyIsDefault = EmptyIsDefault;
	this->SuperType = SuperType;
	this->SelectedId = InitialType;
	auto Icon = EditorUI::Instance->ObjectIcons.GetObjectIcon(SuperType);
	UpdateObjectClass();
	Init(Icon, 0.5f, Width);
}

engine::editor::AssetSelector::~AssetSelector()
{
	if (SearchBackground)
	{
		delete SearchBackground;
	}
}

void engine::editor::AssetSelector::Tick()
{
	if (SearchBackground)
		UpdateSearchSize();
	if (RemoveSearchList)
	{
		ParentWindow->UI.ButtonEvents.push_back(kui::UIManager::ButtonEvent([this]()
		{
			RemoveSearchResults();
		}, nullptr, 0));
	}
	if (AssetPath->GetIsEdited())
	{
		if (!SearchBackground)
		{
			RemoveSearchList = false;
			if (!SearchBackground)
				SearchBackground = new UIScrollBox(false, 0, true);
			UpdateSearchResults();
			UpdateSearchSize();
		}

		if (LastEnteredText != AssetPath->GetText())
		{
			ParentWindow->UI.ButtonEvents.push_back(kui::UIManager::ButtonEvent([this]()
			{
				SearchBackground->UpdateElement();
				SearchBackground->RedrawElement();
				UpdateSearchResults();
			}, nullptr, 0));
			LastEnteredText = AssetPath->GetText();
			UpdateSearchSize();
		}
	}
	else if (SearchBackground
		&& (!ParentWindow->UI.HoveredBox
			|| (!ParentWindow->UI.HoveredBox->IsChildOf(SearchBackground)
				&& !ParentWindow->UI.HoveredBox->IsChildOf(SearchBackground->GetScrollBarBackground()))))
	{
		ParentWindow->UI.ButtonEvents.push_back(kui::UIManager::ButtonEvent([this]()
		{
			RemoveSearchList = true;
		}, nullptr, 0));
	}

	if (SearchBackground && ChangedText && !Window::GetActiveWindow()->Input.IsLMBDown)
	{
		auto NewAsset = AssetRef::FromName(AssetPath->GetText(), SelectedAsset.Extension);
		if ((!NewAsset.IsValid() && !AssetPath->GetText().empty()) || NewAsset.FilePath == SelectedAsset.FilePath)
		{
			ChangedText = false;
			return;
		}
		SelectedAsset = AssetPath->GetText().empty() ? AssetRef{ .Extension = SelectedAsset.Extension } : NewAsset;
		UpdateSelection();
		ChangedText = false;
		if (this->OnChanged)
			this->OnChanged();
		return;
	}
}

void engine::editor::AssetSelector::Init(string Icon, Vec3f Color, kui::UISize Width)
{
	IconBackground = new UIBackground(true, 0, Color, 32_px);

	AddChild(IconBackground
		->SetCorner(EditorUI::Theme.CornerSize)
		->AddChild((new UIBackground(true, 0, 1, 32_px))
			->SetUseTexture(true, Icon))
		->SetPadding(UISize(), UISize(), UISize(), 5_px));

	float PaddingSize = (5_px).GetScreen().X;

	float BoxSize = Width.GetScreen().X - (32_px).GetScreen().X - PaddingSize;

	UIBox* RightBox = new UIBox(false);
	RightBox->SetMinSize(kui::Vec2f(BoxSize, 0));
	AddChild(RightBox);

	AssetPath = new UITextField(0, EditorUI::Theme.DarkBackground, EditorUI::EditorFont, nullptr);
	AssetPath
		->SetText(file::FileNameWithoutExt(SelectedAsset.FilePath))
		->SetHintText(IsAsset ? (SelectedAsset.Extension + " file") : "Class")
		->SetTextColor(EditorUI::Theme.Text)
		->SetTextSize(11_px)
		->SetMinWidth(UISize::Parent(1));

	AssetPath->OnChanged = [this]() {
		ChangedText = true;
	};

	RightBox->AddChild(AssetPath);

	PathText = new UIText(11_px, EditorUI::Theme.Text, "", EditorUI::EditorFont);
	PathText
		->SetWrapEnabled(true, UISize::Screen(BoxSize - PaddingSize * 2))
		->SetPadding(5_px, 0, 5_px, 0);
	RightBox->AddChild(PathText);
	UpdateSelection();
	UpdateElement();
}

void engine::editor::AssetSelector::UpdateSelection()
{
	if (IsAsset)
	{
		AssetPath->SetText(file::FileNameWithoutExt(SelectedAsset.FilePath));
		PathText->SetText(SelectedAsset.FilePath.empty()
			? (EmptyIsDefault ? "<default>" : "<no file>") : SelectedAsset.FilePath);
	}
	else
	{
		AssetPath->SetText(Obj.Name);
		PathText->SetText(SelectedId == 0
			? (EmptyIsDefault ? "<default>" : "<no class>") : Obj.Path);
	}
}

void engine::editor::AssetSelector::UpdateSearchSize()
{
	float VerticalSize = (200_px).GetScreen().Y;
	float ScreenPos = AssetPath->GetScreenPosition().Y;

	VerticalSize = std::min(1 + ScreenPos, VerticalSize);

	SearchBackground->SetMinSize(SizeVec(AssetPath->GetUsedSize().X, VerticalSize));
	SearchBackground->SetMaxSize(SizeVec(AssetPath->GetUsedSize().X, VerticalSize));
	SearchBackground->SetPosition(AssetPath->GetScreenPosition() - Vec2f(0, VerticalSize));
}

void engine::editor::AssetSelector::RemoveSearchResults()
{
	delete SearchBackground;
	SearchBackground = nullptr;
	RemoveSearchList = false;
}

void engine::editor::AssetSelector::UpdateSearchResults()
{
	ChangedText = false;
	SearchBackground->DeleteChildren();

	if (IsAsset)
	{
		UpdateSearchResultsAsset();
	}
	else
	{
		UpdateSearchResultsClass();
	}

	SearchBackground->UpdateElement();
	SearchBackground->RedrawElement();
}

void engine::editor::AssetSelector::UpdateSearchResultsAsset()
{
	std::map<string, string> FilteredAssets;
	string SearchText = str::Lower(AssetPath->GetText());
	for (const auto& [Name, Path] : resource::LoadedAssets)
	{
		string Ext = Name.substr(Name.find_last_of(".") + 1);

		if (Ext != SelectedAsset.Extension)
			continue;

		string WithoutExt = file::FileNameWithoutExt(Name);

		if (SearchText.empty())
			FilteredAssets.insert({ WithoutExt, Path });
		else if (str::Lower(WithoutExt).find(SearchText) != std::string::npos)
			FilteredAssets.insert({ WithoutExt, Path });
	}

	for (const auto& [Name, Path] : FilteredAssets)
	{
		size_t Found = str::Lower(Name).find(SearchText);

		string First = Name.substr(0, Found), Second = Name.substr(Found, SearchText.size()), Third;
		size_t LastPos = Found + SearchText.size();
		if (LastPos < Name.size())
		{
			Third = Name.substr(LastPos);
		}

		SearchBackground->AddChild((new UIButton(true, 0, EditorUI::Theme.DarkBackground2, [this, Name, Path]() {
			AssetPath->SetText(Name);
			SelectedAsset = AssetRef::FromPath(Path);
			ChangedText = false;
			UpdateSelection();
			RemoveSearchResults();
			if (this->OnChanged)
				OnChanged();
		}))
			->SetMinWidth(UISize::Parent(1))
			->AddChild((new UIText(11_px,
				{
					TextSegment(First, EditorUI::Theme.Text),
					TextSegment(Second, EditorUI::Theme.HighlightText),
					TextSegment(Third, EditorUI::Theme.Text),
				}, EditorUI::EditorFont))
				->SetWrapEnabled(true, UISize::Screen(AssetPath->GetUsedSize().GetScreen().X - (10_px).GetScreen().X))
				->SetPadding(4_px, 4_px, 3_px, 3_px)));
	}
}

void engine::editor::AssetSelector::UpdateSearchResultsClass()
{
	std::map<string, Reflection::ObjectInfo> FilteredAssets;
	string SearchText = str::Lower(AssetPath->GetText());
	for (const auto& [Id, Object] : Reflection::ObjectTypes)
	{
		if (!Object.IsSubclassOf(SuperType))
		{
			continue;
		}

		if (SearchText.empty())
			FilteredAssets.insert({ Object.Name, Object });
		else if (str::Lower(Object.Name).find(SearchText) != std::string::npos)
			FilteredAssets.insert({ Object.Name, Object });
	}

	for (const auto& [Name, Class] : FilteredAssets)
	{
		size_t Found = str::Lower(Name).find(SearchText);

		string First = Name.substr(0, Found), Second = Name.substr(Found, SearchText.size()), Third;
		size_t LastPos = Found + SearchText.size();
		if (LastPos < Name.size())
		{
			Third = Name.substr(LastPos);
		}

		SearchBackground->AddChild((new UIButton(true, 0, EditorUI::Theme.DarkBackground2, [this, Name, Class]() {
			AssetPath->SetText(Name);
			SelectedId = Class.TypeID;
			Obj = Class;
			ChangedText = false;
			UpdateSelection();
			RemoveSearchResults();
			if (this->OnChanged)
				OnChanged();
		}))
			->SetMinWidth(UISize::Parent(1))
			->AddChild((new UIText(11_px,
				{
					TextSegment(First, EditorUI::Theme.Text),
					TextSegment(Second, EditorUI::Theme.HighlightText),
					TextSegment(Third, EditorUI::Theme.Text),
				}, EditorUI::EditorFont))
				->SetWrapEnabled(true, UISize::Screen(AssetPath->GetUsedSize().GetScreen().X - (10_px).GetScreen().X))
				->SetPadding(4_px, 4_px, 3_px, 3_px)));
	}
}

void engine::editor::AssetSelector::UpdateObjectClass()
{
	auto Found = Reflection::ObjectTypes.find(this->SelectedId);

	if (Found != Reflection::ObjectTypes.end())
	{
		Obj = Found->second;
	}
	else
	{
		Obj = {};
	}
}
