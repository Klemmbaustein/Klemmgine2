#ifdef EDITOR
#include "EditorUI.h"
#include <Engine/Engine.h>
#include <Engine/Input.h>
#include <Engine/MainThread.h>
#include <Engine/Subsystem/VideoSubsystem.h>
#include <Engine/File/Resource.h>
#include <Core/Error/StackTrace.h>
#include "DropdownMenu.h"
#include <filesystem>
#include <fstream>
#include "Windows/MessageWindow.h"
#include "Elements/DroppableBox.h"
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
EditorPanel* EditorUI::FocusedPanel = nullptr;

static std::map<engine::string, kui::Vec3f> FileNameColors =
{
	{ "", kui::Vec3f(0.5f) },
	{ "png", kui::Vec3f(0.5f, 0.1f, 0.8f) },
	{ "kmt", kui::Vec3f(0.3f, 0.8f, 0.1f) },
	{ "kts", kui::Vec3f(0.6f, 0.1f, 0.3f) },
	{ "kmdl", kui::Vec3f(0.2f, 0.4f, 0.8f) },
	{ "dir/", kui::Vec3f(0.8f, 0.5f, 0) },
};
static std::map<engine::string, engine::string> FileNameIcons =
{
	{ "", "Engine/Editor/Assets/Document.png" },
	{ "png", "Engine/Editor/Assets/Texture.png" },
	{ "kmt", "Engine/Editor/Assets/Material.png" },
	{ "kts", "" },
	{ "kmdl", "Engine/Editor/Assets/Model.png" },
	{ "dir/", "Engine/Editor/Assets/Folder.png" },
};

void engine::editor::EditorUI::StartAssetDrag(DraggedItem With)
{
	if (DraggedBox)
		return;

	CurrentDraggedItem = With;
	CurrentDraggedItem.IsAsset = true;
	auto* NewDragElement = new ItemBrowserButton();
	NewDragElement->UpdateElement();
	NewDragElement->SetBackgroundColor(EditorUI::Theme.DarkBackground2);
	NewDragElement->SetImage(With.Icon);
	NewDragElement->SetColor(With.Color);
	NewDragElement->SetName(With.Name);

	DraggedBox = NewDragElement;
}

void engine::editor::EditorUI::StartDrag(kui::UIBox* bx, bool Centered)
{
	if (DraggedBox)
		return;
	CurrentDraggedItem = DraggedItem();
	CurrentDraggedItem.Centered = Centered;
	CurrentDraggedItem.IsAsset = false;
	DraggedBox = bx;
}

std::pair<engine::string, kui::Vec3f> engine::editor::EditorUI::GetExtIconAndColor(string Extension)
{
	if (!FileNameIcons.contains(Extension))
		Extension = "";

	return { FileNameIcons[Extension], FileNameColors[Extension] };
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
	Instance->StatsBarElement->RedrawElement();
}

engine::editor::EditorUI::EditorUI()
{
	using namespace kui;
	using namespace subsystem;

	Instance = this;

	VideoSubsystem* VideoSystem = Engine::GetSubsystem<VideoSubsystem>();
	VideoSystem->OnResizedCallbacks.insert({ this, [this](kui::Vec2ui NewSize)
		{
			UpdateBackgrounds();
			RootPanel->ShouldUpdate = true;
		} });

	UpdateTheme(VideoSystem->MainWindow);

	if (!MonospaceFont)
	{
		EditorFont = VideoSystem->DefaultFont;
		MonospaceFont = new Font("Engine/Editor/Assets/EditorMono.ttf");
	}

	UIBackground* Root = new UIBackground(false, -1, Theme.Background, 2);
	Root
		->SetMinSize(2)
		->SetMaxSize(2);

	MenuBar = new UIBackground(true, 0, Theme.LightBackground, SizeVec(UISize::Parent(1), UISize::Pixels(MenuBarSize)));
	MenuBar
		->SetVerticalAlign(UIBox::Align::Centered);
	Root->AddChild(MenuBar);

	AddMenuBarItem("File", { "New", "Open Project", "Exit" });
	AddMenuBarItem("Edit", { "Undo", "Settings", "Project settings" });
	AddMenuBarItem("Scene", { "Save", "Open", "Show collision view" });
	AddMenuBarItem("Window", { "Save layout", "Load layout" });
	AddMenuBarItem("Help", { "About", "Source code" });

	MainBackground = new UIBox(true, 0);

	Root->AddChild(MainBackground);

	StatsBarElement = new StatusBarElement();
	StatusBar = new UIBackground(true, 0, Theme.DarkBackground,
		SizeVec(UISize::Parent(1), UISize::Pixels(StatusBarSize)));

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

	SetStatusMainThread("Editor loaded", StatusType::Info);
	input::ShowMouseCursor = true;

	Update();
}

engine::editor::EditorUI::~EditorUI()
{
	using namespace subsystem;

	VideoSubsystem* VideoSystem = Engine::GetSubsystem<VideoSubsystem>();

	VideoSystem->OnResizedCallbacks.erase(this);
	delete RootPanel;
	delete MainBackground->GetAbsoluteParent();

	VideoSystem->OnResized();
	input::ShowMouseCursor = false;
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
	using namespace kui;

	if (DraggedBox)
	{
		if (CurrentDraggedItem.Centered)
			DraggedBox->SetPosition(DraggedBox->GetParentWindow()->Input.MousePosition - DraggedBox->GetUsedSize().GetScreen() / 2);
		else
			DraggedBox->SetPosition(DraggedBox->GetParentWindow()->Input.MousePosition - Vec2f(0, DraggedBox->GetUsedSize().GetScreen().Y));
		Window::GetActiveWindow()->CurrentCursor = Window::Cursor::Default;

		// Don't hover anything new when dragging something.
		Window::GetActiveWindow()->UI.HoveredBox = nullptr;
		Window::GetActiveWindow()->UI.NewHoveredBox = nullptr;

		if (!input::IsLMBDown || input::IsRMBDown)
		{
			if (CurrentDraggedItem.IsAsset)
			{
				DroppableBox* DropTo = DroppableBox::GetBoxAtCursor();
				if (DropTo && DropTo->OnDrop)
				{
					DropTo->OnDrop(CurrentDraggedItem);
				}
			}
			delete DraggedBox;
			DraggedBox = nullptr;
		}
	}

	EditorPanel::UpdateAllPanels();
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

void engine::editor::EditorUI::AddMenuBarItem(std::string Name, std::vector<std::string> Options)
{
	auto* btn = new MenuBarButton();
	btn->SetName(Name);

	std::vector<DropdownMenu::Option> DropdownOptions;

	for (auto& i : Options)
	{
		DropdownOptions.push_back(DropdownMenu::Option{
			.Name = i,
			});
	}

	btn->button->OnClicked = [btn, DropdownOptions]()
		{
			new DropdownMenu(DropdownOptions, btn->GetPosition());
		};
	MenuBar->AddChild(btn);
}

#endif