#ifdef EDITOR
#include "EditorUI.h"
#include <Engine/Engine.h>
#include <Engine/Subsystem/VideoSubsystem.h>
#include <kui/Resource.h>
#include <MenuBar.kui.hpp>
#include "DropdownMenu.h"

#include "Viewport.h"
#include "AssetBrowser.h"

const float StatusBarSize = 24;
const float MenuBarSize = 24;
engine::editor::EditorUI* engine::editor::EditorUI::Instance = nullptr;
kui::Font* engine::editor::EditorUI::EditorFont = nullptr;
engine::editor::EditorTheme engine::editor::EditorUI::Theme;

std::vector<engine::string> MenuBarEntries = { "File", "Edit", "Scene", "Window", "Help"};

engine::editor::EditorUI::EditorUI()
{
	using namespace kui;
	using namespace subsystem;

	Instance = this;
	
	VideoSubsystem* VideoSystem = Engine::GetSubsystem<VideoSubsystem>();
	VideoSystem->OnResizedCallbacks.push_back([this](kui::Vec2ui NewSize) {
		UpdateBackgrounds();
		RootPanel->ShouldUpdate = true;
		});

	string FontResourcePath = "res:DefaultFont.ttf";
	string WindowsFontPath = "file:C:/Windows/Fonts/seguisb.ttf";
	if (resource::FileExists(WindowsFontPath))
	{
		FontResourcePath = WindowsFontPath;
	}

	EditorFont = new Font(FontResourcePath);
	VideoSystem->MainWindow->Markup.SetDefaultFont(EditorFont);

	UIBackground* Root = new UIBackground(false, -1, Theme.Background, 2);
	Root
		->SetMinSize(2)
		->SetMaxSize(2);

	MenuBar = new UIBackground(true, 0, Theme.LightBackground, MenuBarSize);
	MenuBar
		->SetVerticalAlign(UIBox::Align::Centered)
		->SetSizeMode(UIBox::SizeMode::PixelRelative)
		->SetTryFill(true);

	for (auto& i : MenuBarEntries)
	{
		auto* btn = new MenuBarButton();
		btn->SetName(i);
		btn->button->OnClicked = [this, btn]()
			{
				new DropdownMenu({ DropdownMenu::Option{
					.OnClicked = []() {abort(); },
					.Name = "Testing",
					},
					DropdownMenu::Option{
					.OnClicked = []() {abort(); },
					.Name = "Testing 2",
					} }, btn->button->GetPosition());
			};
		MenuBar->AddChild(btn);
	}
	Root->AddChild(MenuBar);

	MainBackground = new UIBox(true, 0);

	Root->AddChild(MainBackground);

	StatsBarElement = new StatusBarElement();
	StatusBar = new UIBackground(true, 0, Theme.DarkBackground, StatusBarSize);
	StatusBar
		->SetSizeMode(UIBox::SizeMode::PixelRelative)
		->SetTryFill(true);
	Root->AddChild(StatusBar
		->AddChild(StatsBarElement));

	UpdateBackgrounds();

	RootPanel = new EditorPanel("root");
	RootPanel->AddChild((new Viewport())->SetWidth(0.85f), EditorPanel::Align::Horizontal);
	RootPanel->AddChild(new AssetBrowser(), EditorPanel::Align::Horizontal);
}

void engine::editor::EditorUI::Update()
{
	RootPanel->UpdatePanel();

	DropdownMenu::UpdateDropdowns();
}

void engine::editor::EditorUI::UpdateBackgrounds()
{
	using namespace kui;
	MainBackground->SetMinSize(Vec2f(
		2,
		2 - UIBox::PixelSizeToScreenSize(Vec2f(0, StatusBarSize + MenuBarSize), MainBackground->GetParentWindow()).Y)
	);
	MainBackground->GetParent()->UpdateElement();
}

#endif