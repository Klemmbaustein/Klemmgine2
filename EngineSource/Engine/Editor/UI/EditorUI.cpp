#ifdef EDITOR
#include "EditorUI.h"
#include <Engine/Engine.h>
#include <Engine/Input.h>
#include <Engine/MainThread.h>
#include <Engine/Subsystem/VideoSubsystem.h>
#include <Engine/Editor/Assets.h>
#include "DropdownMenu.h"
#include <kui/Resource.h>
#include <filesystem>
#include <fstream>

#include "Panels/Viewport.h"
#include "Panels/AssetBrowser.h"
#include "Panels/ClassBrowser.h"
#include "Panels/ConsolePanel.h"
#include "Panels/MessagePanel.h"
#include "Panels/ObjectListPanel.h"
#include "Panels/PropertyPanel.h"

#include <ItemBrowser.kui.hpp>
#include <MenuBar.kui.hpp>

using namespace engine::editor;

static const float StatusBarSize = 24;
static const float MenuBarSize = 24;

EditorUI* EditorUI::Instance = nullptr;
kui::Font* EditorUI::EditorFont = nullptr;
kui::Font* EditorUI::MonospaceFont = nullptr;
EditorTheme EditorUI::Theme;
engine::string EditorUI::DefaultFontName = "res:DefaultFont.ttf";
EditorPanel* EditorUI::FocusedPanel = nullptr;

std::vector<engine::string> MenuBarEntries = { "File", "Edit", "Scene", "Window", "Help" };

void engine::editor::EditorUI::StartDrag(DraggedItem With)
{
	if (DraggedBox)
		return;

	CurrentDraggedItem = With;

	auto* NewDragElement = new ItemBrowserButton();
	NewDragElement->UpdateElement();
	NewDragElement->SetBackgroundColor(EditorUI::Theme.DarkBackground2);
	NewDragElement->SetImage(With.Icon);
	NewDragElement->SetColor(With.Color);
	NewDragElement->SetName(With.Name);

	DraggedBox = NewDragElement;
}

void engine::editor::EditorUI::SetStatusMessage(string NewMessage, StatusType Type)
{
	if (thread::IsMainThread)
	{
		SetStatusMainThread(NewMessage, Type);
	}
	else
	{
		thread::ExecuteOnMainThread([NewMessage, Type]()
		{
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
	assets::ScanForAssets();

	VideoSubsystem* VideoSystem = Engine::GetSubsystem<VideoSubsystem>();
	VideoSystem->OnResizedCallbacks.insert({ this, [this](kui::Vec2ui NewSize)
		{
			UpdateBackgrounds();
			RootPanel->ShouldUpdate = true;
		} });

	UpdateTheme(VideoSystem->MainWindow);

	string WindowsFontPath = "file:C:/Windows/Fonts/seguisb.ttf";
	if (resource::FileExists(WindowsFontPath))
	{
		DefaultFontName = WindowsFontPath;
	}

	EditorFont = new Font(DefaultFontName);
	VideoSystem->MainWindow->Markup.SetDefaultFont(EditorFont);
	MonospaceFont = new Font("Engine/Editor/Assets/EditorMono.ttf");

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
		btn->button->OnClicked = [btn]()
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

	EditorPanel* Left = new EditorPanel("panel");
	Left->AddChild(new AssetBrowser(), EditorPanel::Align::Tabs);
	Left->AddChild(new ClassBrowser(), EditorPanel::Align::Tabs);

	RootPanel->AddChild(Left->SetWidth(0.15f), EditorPanel::Align::Horizontal);

	EditorPanel* Center = new EditorPanel("panel");
	EditorPanel* LowerPanel = new EditorPanel("panel");
	Center->AddChild(LowerPanel->SetWidth(0.2f), EditorPanel::Align::Vertical);
	LowerPanel->AddChild(new ConsolePanel(), EditorPanel::Align::Tabs);
	LowerPanel->AddChild(new MessagePanel(), EditorPanel::Align::Tabs);

	auto vp = new Viewport();

	Center->AddChild(vp->SetWidth(1.8f), EditorPanel::Align::Vertical);
	RootPanel->AddChild(Center->SetWidth(0.7f), EditorPanel::Align::Horizontal);
	EditorPanel* Right = new EditorPanel("panel");
	Right->AddChild(new PropertyPanel(), EditorPanel::Align::Vertical);
	Right->AddChild(new ObjectListPanel(), EditorPanel::Align::Vertical);
	RootPanel->AddChild(Right->SetWidth(0.15f), EditorPanel::Align::Horizontal);

	FocusedPanel = vp;

	SetStatusMainThread("Ready", StatusType::Info);
}

engine::string engine::editor::EditorUI::CreateAsset(string Path, string Name, string Extension)
{
	string FileName = Path + "/" + Name;
	string FileNumber = "";
	int32 FileNumberValue = 0;

	while (std::filesystem::exists(FileName + FileNumber + "." + Extension))
	{
		FileNumber = str::Format(" (%i)", ++FileNumberValue);
	}

	string NewPath = FileName + FileNumber + "." + Extension;

	std::ofstream File = std::ofstream(NewPath);
	File.close();

	return NewPath;
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
	if (DraggedBox)
	{
		DraggedBox->SetPosition(DraggedBox->GetParentWindow()->Input.MousePosition - DraggedBox->GetUsedSize() / 2);

		if (!input::IsLMBDown || input::IsRMBDown)
		{
			delete DraggedBox;
			DraggedBox = nullptr;
		}
	}

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