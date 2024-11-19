#ifdef EDITOR
#include "EditorUI.h"
#include <Engine/Engine.h>
#include <Engine/MainThread.h>
#include <Engine/Subsystem/VideoSubsystem.h>
#include <kui/Resource.h>
#include <MenuBar.kui.hpp>
#include "DropdownMenu.h"

#include "Viewport.h"
#include "AssetBrowser.h"
#include "ConsolePanel.h"
#include "MessagePanel.h"
#include "ObjectListPanel.h"

const float StatusBarSize = 24;
const float MenuBarSize = 24;
engine::editor::EditorUI* engine::editor::EditorUI::Instance = nullptr;
kui::Font* engine::editor::EditorUI::EditorFont = nullptr;
kui::Font* engine::editor::EditorUI::MonospaceFont = nullptr;
engine::editor::EditorTheme engine::editor::EditorUI::Theme;
engine::string engine::editor::EditorUI::DefaultFontName = "res:DefaultFont.ttf";
engine::editor::EditorPanel* engine::editor::EditorUI::FocusedPanel = nullptr;

std::vector<engine::string> MenuBarEntries = { "File", "Edit", "Scene", "Window", "Help" };

void engine::editor::EditorUI::SetStatusMessage(string NewMessage, StatusType Type)
{
	if (thread::IsMainThread)
	{
		SetStatusMainThread(NewMessage, Type);
	}
	else
	{
		thread::ExecuteOnMainThread([NewMessage, Type]() {
			SetStatusMainThread(NewMessage, Type);
			});
	}
}

void engine::editor::EditorUI::SetStatusMainThread(string NewMessage, StatusType Type)
{
	static std::vector<engine::string> IconTypes = { "Info.png", "Warn.png", "Error.png" };

	Instance->CurrentStatus = NewMessage;
	Instance->StatsBarElement->SetText(NewMessage);
	Instance->StatsBarElement->SetIcon("Engine/Editor/Assets/" + IconTypes[int(Type)]);
}

engine::editor::EditorUI::EditorUI()
{
	using namespace kui;
	using namespace subsystem;

	Instance = this;


	VideoSubsystem* VideoSystem = Engine::GetSubsystem<VideoSubsystem>();
	VideoSystem->OnResizedCallbacks.push_back([this](kui::Vec2ui NewSize)
		{
			UpdateBackgrounds();
			RootPanel->ShouldUpdate = true;
		});

	UpdateTheme(VideoSystem->MainWindow);

	string WindowsFontPath = "file:C:/Windows/Fonts/seguisb.ttf";
	if (resource::FileExists(WindowsFontPath))
	{
		DefaultFontName = WindowsFontPath;
	}

	EditorFont = new Font(DefaultFontName);
	VideoSystem->MainWindow->Markup.SetDefaultFont(EditorFont);
	MonospaceFont = new Font("res:EditorMono.ttf");

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
					} }, btn->GetPosition());
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

	RootPanel->AddChild((new AssetBrowser())->SetWidth(0.15f), EditorPanel::Align::Horizontal);

	EditorPanel* Center = new EditorPanel("panel");
	EditorPanel* LowerPanel = new EditorPanel("panel");
	Center->AddChild(LowerPanel->SetWidth(0.2f), EditorPanel::Align::Vertical);
	LowerPanel->AddChild(new ConsolePanel(), EditorPanel::Align::Tabs);
	LowerPanel->AddChild(new MessagePanel(), EditorPanel::Align::Tabs);

	auto vp = new Viewport();

	Center->AddChild(vp->SetWidth(1.8f), EditorPanel::Align::Vertical);
	RootPanel->AddChild(Center->SetWidth(0.72f), EditorPanel::Align::Horizontal);
	RootPanel->AddChild((new ObjectListPanel())->SetWidth(0.15f), EditorPanel::Align::Horizontal);

	FocusedPanel = vp;

	SetStatusMainThread("Ready", StatusType::Info);
}

void engine::editor::EditorUI::UpdateTheme(kui::Window* Target)
{
	Target->Markup.SetGlobal("Color_Text", Theme.Text);
	Target->Markup.SetGlobal("Color_Background", Theme.Background);
	Target->Markup.SetGlobal("Color_DarkBackground", Theme.DarkBackground);
	Target->Markup.SetGlobal("Color_DarkBackground2", Theme.DarkBackground2);
	Target->Markup.SetGlobal("Color_LightBackground", Theme.LightBackground);
	Target->Markup.SetGlobal("Color_BackgroundHighlight", Theme.BackgroundHighlight);
	Target->Markup.SetGlobal("Color_Highlight1", Theme.Highlight1);
	Target->Markup.SetGlobal("Color_HighlightDark", Theme.HighlightDark);
	Target->Markup.SetGlobal("Color_Highlight2", Theme.Highlight2);
	Target->Markup.SetGlobal("Color_HighlightText", Theme.HighlightText);

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