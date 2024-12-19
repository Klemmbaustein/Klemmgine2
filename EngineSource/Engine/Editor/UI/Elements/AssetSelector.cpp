#include "AssetSelector.h"
#include <kui/Window.h>
#include <Engine/Log.h>
#include <Engine/Editor/Assets.h>
#include <Engine/Error/EngineAssert.h>
#include <Engine/Editor/UI/EditorUI.h>
#include <Engine/File/FileUtil.h>
using namespace kui;

engine::editor::AssetSelector::AssetSelector(AssetRef InitialValue, float Width, std::function<void()> OnChanged)
	: DroppableBox(true)
{
	SelectedAsset = InitialValue;
	IconBackground = new UIBackground(true, 0, 1, 24);

	AddChild(IconBackground
		->SetSizeMode(UIBox::SizeMode::PixelRelative)
		->SetPadding(0, 0, 0, 5)
		->SetPaddingSizeMode(UIBox::SizeMode::PixelRelative));

	IconBackground->UpdateElement();

	float PaddingSize = PixelSizeToScreenSize(5, ParentWindow).X;

	float BoxSize = Width - IconBackground->GetUsedSize().X - PaddingSize;

	UIBox* RightBox = new UIBox(false);
	RightBox->SetMinSize(kui::Vec2f(BoxSize, 0));
	AddChild(RightBox);

	AssetPath = new UITextField(0, EditorUI::Theme.DarkBackground, EditorUI::EditorFont, nullptr);
	AssetPath
		->SetText(file::FileNameWithoutExt(SelectedAsset.FilePath))
		->SetHintText(SelectedAsset.Extension + " file")
		->SetTextColor(EditorUI::Theme.Text)
		->SetTextSize(11)
		->SetTextSizeMode(UIBox::SizeMode::PixelRelative)
		->SetTryFill(true);

	RightBox->AddChild(AssetPath);

	PathText = new UIText(11, EditorUI::Theme.Text, "", EditorUI::EditorFont);
	PathText
		->SetWrapEnabled(true, BoxSize - PaddingSize * 2, UIBox::SizeMode::ScreenRelative)
		->SetTextSizeMode(UIBox::SizeMode::PixelRelative)
		->SetPadding(5, 0, 5, 0)
		->SetPaddingSizeMode(UIBox::SizeMode::PixelRelative);
	RightBox->AddChild(PathText);
	UpdateSelection();

	OnDrop = [this](EditorUI::DraggedItem Item)
		{
			SelectedAsset = AssetRef::FromPath(Item.Path);
		};
	UpdateElement();
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
}

void engine::editor::AssetSelector::UpdateSelection()
{
	PathText->SetText(SelectedAsset.FilePath.empty() ? "<no file>" : SelectedAsset.FilePath);
}

void engine::editor::AssetSelector::UpdateSearchSize()
{
	float Offset = 0;

	if (CurrentScrollObject)
	{
		Offset = CurrentScrollObject->Percentage;
	}

	float VerticalSize = PixelSizeToScreenSize(200, ParentWindow).Y;
	SearchBackground->SetMinSize(Vec2f(AssetPath->GetUsedSize().X, VerticalSize));
	SearchBackground->SetMaxSize(Vec2f(AssetPath->GetUsedSize().X, VerticalSize));
	SearchBackground->SetPosition(AssetPath->GetScreenPosition() - Vec2f(0, VerticalSize - Offset));
}

void engine::editor::AssetSelector::RemoveSearchResults()
{
	delete SearchBackground;
	SearchBackground = nullptr;
	RemoveSearchList = false;
}

void engine::editor::AssetSelector::UpdateSearchResults()
{
	SearchBackground->DeleteChildren();

	std::map<string, string> FilteredAssets;
	string SearchText = str::Lower(AssetPath->GetText());

	for (const auto& [Name, Path] : assets::LoadedAssets)
	{
		string Ext = Name.substr(Name.find_last_of(".") + 1);

		if (Ext != SelectedAsset.Extension)
			continue;

		string WithoutExt = file::FileNameWithoutExt(Name);
		if (str::Lower(WithoutExt).find(SearchText) != std::string::npos)
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

		SearchBackground->AddChild((new UIButton(true, 0, EditorUI::Theme.DarkBackground2, [this, Name, Path]()
			{
				AssetPath->SetText(Name);
				RemoveSearchResults();
				SelectedAsset = AssetRef::FromPath(Path);
				OnChanged();
				UpdateSelection();
			}))
			->SetTryFill(true)
			->AddChild((new UIText(11,
			{
				TextSegment(First, EditorUI::Theme.Text),
				TextSegment(Second, Vec3f(1, 0.5f, 0.0f)),
				TextSegment(Third, EditorUI::Theme.Text),
			}, EditorUI::EditorFont))
			->SetTextSizeMode(UIBox::SizeMode::PixelRelative)
			->SetWrapEnabled(true, AssetPath->GetUsedSize().X - PixelSizeToScreenSize(10, ParentWindow).X, UIBox::SizeMode::ScreenRelative)
			->SetPadding(4, 4, 3, 3)
			->SetPaddingSizeMode(UIBox::SizeMode::PixelRelative)));
	}
}
