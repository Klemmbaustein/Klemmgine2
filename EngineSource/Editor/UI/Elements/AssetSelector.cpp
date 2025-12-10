#include "AssetSelector.h"
#include <kui/Window.h>
#include <Engine/File/Resource.h>
#include <Editor/UI/EditorUI.h>
#include <Core/File/FileUtil.h>
using namespace kui;

engine::editor::AssetSelector::AssetSelector(AssetRef InitialValue, kui::UISize Width, std::function<void()> OnChanged)
	: DroppableBox(true, [this](EditorUI::DraggedItem itm)
{
	auto ref = AssetRef::FromPath(itm.Path);
	if (ref.Extension == this->SelectedAsset.Extension || this->SelectedAsset.Extension.empty())
	{
		SelectedAsset = ref;
		this->OnChanged();
		UpdateSelection();
	}

})
{
	SelectedAsset = InitialValue;
	auto c = EditorUI::GetExtIconAndColor(InitialValue.Extension);
	IconBackground = new UIBackground(true, 0, c.second, 32_px);

	AddChild(IconBackground
		->SetCorner(EditorUI::Theme.CornerSize)
		->AddChild((new UIBackground(true, 0, 1, 32_px))
			->SetUseTexture(true, c.first))
		->SetPadding(UISize(), UISize(), UISize(), 5_px));

	float PaddingSize = (5_px).GetScreen().X;

	float BoxSize = Width.GetScreen().X - (32_px).GetScreen().X - PaddingSize;

	UIBox* RightBox = new UIBox(false);
	RightBox->SetMinSize(kui::Vec2f(BoxSize, 0));
	AddChild(RightBox);

	AssetPath = new UITextField(0, EditorUI::Theme.DarkBackground, EditorUI::EditorFont, nullptr);
	AssetPath
		->SetText(file::FileNameWithoutExt(SelectedAsset.FilePath))
		->SetHintText(SelectedAsset.Extension + " file")
		->SetTextColor(EditorUI::Theme.Text)
		->SetTextSize(11_px)
		->SetMinWidth(UISize::Parent(1));

	AssetPath->OnChanged = [this]()
	{
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
	if (ChangedText && !Window::GetActiveWindow()->Input.IsLMBDown)
	{
		auto NewAsset = AssetRef::FromName(AssetPath->GetText(), SelectedAsset.Extension);
		if (NewAsset.FilePath.empty())
			return;

		SelectedAsset = AssetPath->GetText().empty() ? AssetRef{ .Extension = SelectedAsset.Extension } : NewAsset;
		if (this->OnChanged)
			this->OnChanged();
		UpdateSelection();
		ChangedText = false;
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
	AssetPath->SetText(file::FileNameWithoutExt(SelectedAsset.FilePath));
	PathText->SetText(SelectedAsset.FilePath.empty() ? "<no file>" : SelectedAsset.FilePath);
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

		SearchBackground->AddChild((new UIButton(true, 0, EditorUI::Theme.DarkBackground2, [this, Name, Path]()
		{
			AssetPath->SetText(Name);
			SelectedAsset = AssetRef::FromPath(Path);
			ChangedText = false;
			if (this->OnChanged)
				OnChanged();
			UpdateSelection();
			RemoveSearchResults();
		}))
			->SetMinWidth(UISize::Parent(1))
			->AddChild((new UIText(11_px,
				{
					TextSegment(First, EditorUI::Theme.Text),
					TextSegment(Second, Vec3f(1, 0.5f, 0.0f)),
					TextSegment(Third, EditorUI::Theme.Text),
				}, EditorUI::EditorFont))
				->SetWrapEnabled(true, UISize::Screen(AssetPath->GetUsedSize().GetScreen().X - (10_px).GetScreen().X))
				->SetPadding(4_px, 4_px, 3_px, 3_px)));
	}
	SearchBackground->UpdateElement();
	SearchBackground->RedrawElement();
}