#include "DropdownMenu.h"
#include "EditorUI.h"
#include "Elements/DroppableBox.h"
#include "Panels/AssetBrowser.h"
#include "Panels/ClassBrowser.h"
#include "Panels/ConsolePanel.h"
#include "Panels/MessagePanel.h"
#include "Panels/ObjectListPanel.h"
#include "Panels/PropertyPanel.h"
#include "Panels/ScenePanel.h"
#include "Panels/ScriptEditorPanel.h"
#include "Panels/Viewport.h"
#include "Layout/SaveLayout.h"
#include "Windows/AboutWindow.h"
#include "Windows/BuildWindow.h"
#include "Windows/SettingsWindow.h"
#include <Editor/Editor.h>
#include <Editor/Settings/EditorSettings.h>
#include <Engine/Engine.h>
#include <Engine/File/Resource.h>
#include <Engine/Graphics/VideoSubsystem.h>
#include <Engine/Input.h>
#include <Engine/Internal/PlatformGraphics.h>
#include <Engine/MainThread.h>
#include <Engine/Objects/MeshObject.h>
#include <ItemBrowser.kui.hpp>
#include <MenuBar.kui.hpp>
#include <filesystem>
using namespace engine::editor;
using namespace engine::subsystem;
using namespace engine;
using namespace kui;

static const float StatusBarSize = 24;
static const float MenuBarSize = 24;

EditorUI* EditorUI::Instance = nullptr;
Font* EditorUI::EditorFont = nullptr;
Font* EditorUI::MonospaceFont = nullptr;
EditorTheme EditorUI::Theme;
EditorPanel* EditorUI::FocusedPanel = nullptr;

static std::map<engine::string, Vec3f> FileNameColors =
{
	{ "", Vec3f(0.5f) },
	{ "png", Vec3f(0.4f, 0.2f, 0.6f) },
	{ "kmt", Vec3f(0.4f, 0.7f, 0.3f) },
	{ "kts", Vec3f(0.6f, 0.1f, 0.3f) },
	{ "kmdl", Vec3f(0.3f, 0.4f, 0.7f) },
	{ "dir/", Vec3f(0.7f, 0.5f, 0) },
};
static std::map<string, string> FileNameIcons =
{
	{ "", "Document.png" },
	{ "png", "Texture.png" },
	{ "kmt", "Material.png" },
	{ "kts", "" },
	{ "kmdl", "Model.png" },
	{ "vert", "VertexShader.png" },
	{ "frag", "FragmentShader.png" },
	{ "ds", "Code.png" },
	{ "dir/", "Folder.png" },
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

	auto& Icon = FileNameIcons[Extension];

	return { Icon.empty() ? "" : EditorUI::Asset(Icon), FileNameColors[Extension] };
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
	Instance->StatsBarElement->SetIcon(Asset(IconTypes[int(Type)]));
	Instance->StatsBarElement->RedrawElement();
}

void engine::editor::EditorUI::InitTheme()
{
	Theme.LoadFromFile(Settings::GetInstance()->Interface.GetSetting("theme", "Dark").GetString());
}

engine::editor::EditorUI::EditorUI()
{
	Instance = this;

	InitTheme();

	ObjectIcons.AddObjectIcon(Asset("Model.png"), MeshObject::ObjectType);

	VideoSubsystem* VideoSystem = Engine::GetSubsystem<VideoSubsystem>();
	VideoSystem->OnResizedCallbacks.insert({ this, [this](Vec2ui NewSize) {
		UpdateBackgrounds();
		RootPanel->ShouldUpdate = true;
		RootPanel->UpdatePanel();
	} });

	UpdateTheme(VideoSystem->MainWindow, false);

	if (!MonospaceFont)
	{
		EditorFont = VideoSystem->DefaultFont;
		MonospaceFont = new Font(Asset("EditorMono.ttf"));
		MonospaceFont->CharacterSize *= 1.1f;
		VideoSystem->MainWindow->Markup.AddFont("mono", MonospaceFont);
	}

	Root = new UIBackground(false, -1, Theme.DarkBackground, 2);
	Root
		->SetMinSize(2)
		->SetMaxSize(2);

	MenuBar = new UIBackground(true, 0, Theme.DarkBackground, SizeVec(UISize::Parent(1), UISize::Pixels(MenuBarSize)));
	MenuBar
		->SetVerticalAlign(UIBox::Align::Centered);
	Root->AddChild(MenuBar);

	MainBackground = new UIBox(true, 0);

	Root->AddChild(MainBackground);

	StatsBarElement = new StatusBarElement();
	StatusBar = new UIBackground(true, 0, Theme.DarkBackground,
		SizeVec(UISize::Parent(1), UISize::Pixels(StatusBarSize)));

	Root->AddChild(StatusBar
		->AddChild(StatsBarElement));

	UpdateBackgrounds();

	RootPanel = new EditorPanel("root");

	string LayoutFile = GetLayoutConfigPath() + "/lastLayout.k2b";

	if (!std::filesystem::exists(LayoutFile))
	{
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
		auto Properties = new EditorPanel("panel");
		Right->AddChild(Properties, EditorPanel::Align::Vertical);

		Properties->AddChild(new PropertyPanel(), EditorPanel::Align::Tabs);
		Properties->AddChild(new ScenePanel(), EditorPanel::Align::Tabs);

		Right->AddChild(new ObjectListPanel(), EditorPanel::Align::Vertical);
		RootPanel->AddChild(Right->SetWidth(0.15f), EditorPanel::Align::Horizontal);

		vp->AddChild(new ScriptEditorPanel(), EditorPanel::Align::Tabs, false);
	}
	else
	{
		layout::LoadLayout(RootPanel, LayoutFile);
	}

	FocusedPanel = Viewport::Current;

	SetStatusMainThread("Editor loaded", StatusType::Info);
	input::ShowMouseCursor = true;

	AddMenuBarItem("File",
		{
			DropdownMenu::Option("New", "", Asset("Plus.png")),
			DropdownMenu::Option("Open Project"),
			DropdownMenu::Option("Build Project", "", Asset("Build.png"), []() {
				new BuildWindow();
			}),
			DropdownMenu::Option("Exit", "Alt+F4", Asset("X.png"), []() {
				Engine::Instance->ShouldQuit = true;
			}),
		});

	AddMenuBarItem("Edit",
		{
			DropdownMenu::Option("Undo", "Ctrl+Z", Asset("Undo.png"), []() {
				Viewport::Current->UndoLast();
			}),
			DropdownMenu::Option("Redo", "Ctrl+Y", Asset("Redo.png")),
			DropdownMenu::Option("Settings", "", Asset("Settings.png"), []() {
				new SettingsWindow();
			}),
			DropdownMenu::Option("Project settings"),
		});

	AddMenuBarItem("Scene",
		{
			DropdownMenu::Option("Save", "Alt+S", Asset("Save.png")),
			DropdownMenu::Option("Open", "", Asset("Open.png")),
			DropdownMenu::Option("Run", "F5", Asset("Run.png"), []() {
				Viewport::Current->Run();
			}),
			DropdownMenu::Option{
				.Name = "View",
				.SubMenu = Viewport::Current->GetViewDropdown()
			},
		});
	AddMenuBarItem("Window",
		{
			DropdownMenu::Option("Save layout"),
			DropdownMenu::Option("Load layout"),
		});

	AddMenuBarItem("Help",
		{ DropdownMenu::Option("About", "", Asset("Info.png"), []() {
				new AboutWindow();
			}), DropdownMenu::Option("Source code") }
	);

	Update();
}

engine::editor::EditorUI::~EditorUI()
{
	using namespace subsystem;

	string LayoutPath = GetLayoutConfigPath();

	std::filesystem::create_directories(GetLayoutConfigPath());
	layout::LayoutToFile(RootPanel, LayoutPath + "/lastLayout.k2b");

	VideoSubsystem* VideoSystem = Engine::GetSubsystem<VideoSubsystem>();

	VideoSystem->OnResizedCallbacks.erase(this);
	delete RootPanel;
	delete MainBackground->GetAbsoluteParent();

	VideoSystem->OnResized();
	input::ShowMouseCursor = false;
}

string engine::editor::EditorUI::GetLayoutConfigPath()
{
	return editor::GetEditorPath() + "/Config/Layout/";
}

engine::string engine::editor::EditorUI::CreateAsset(string Path, string Name, string Extension)
{
	if (*Path.rbegin() != '/')
	{
		Path.push_back('/');
	}

	string FileName = Path + Name;
	string FileNumber = "";
	int32 FileNumberValue = 0;

	while (resource::FileExists(FileName + FileNumber + "." + Extension))
	{
		FileNumber = str::Format(" (%i)", ++FileNumberValue);
	}

	string NewPath = FileName + FileNumber + "." + Extension;

	Instance->AssetsProvider->NewFile(NewPath);

	return NewPath;
}

string engine::editor::EditorUI::CreateDirectory(string Path)
{
	while (!Path.empty() && *Path.rbegin() == '/')
	{
		Path.pop_back();
	}

	string FileName = Path;
	string FileNumber = "";
	int32 FileNumberValue = 0;

	while (resource::FileExists(FileName + FileNumber + "/"))
	{
		FileNumber = str::Format(" (%i)", ++FileNumberValue);
	}

	string NewPath = FileName + FileNumber;

	Instance->AssetsProvider->NewDirectory(NewPath);

	return NewPath;
}

void engine::editor::EditorUI::UpdateTheme(kui::Window* Target, bool Full)
{
	Target->Markup.SetGlobal("Color_Text", Theme.Text);
	Target->Markup.SetGlobal("Color_DarkText", Theme.DarkText);
	Target->Markup.SetGlobal("Color_Background", Theme.Background);
	Target->Markup.SetGlobal("Color_DarkBackground", Theme.DarkBackground);
	Target->Markup.SetGlobal("Color_DarkBackground2", Theme.DarkBackground2);
	Target->Markup.SetGlobal("Color_LightBackground", Theme.LightBackground);
	Target->Markup.SetGlobal("Color_DarkBackgroundHighlight", Theme.DarkBackgroundHighlight);
	Target->Markup.SetGlobal("Color_BackgroundHighlight", Theme.BackgroundHighlight);
	Target->Markup.SetGlobal("Color_Highlight1", Theme.Highlight1);
	Target->Markup.SetGlobal("Color_HighlightDark", Theme.HighlightDark);
	Target->Markup.SetGlobal("Color_HighlightText", Theme.HighlightText);
	Target->Markup.SetGlobal("Color_SelectedText", Theme.SelectedText);
	Target->Markup.SetGlobal("Theme_CornerSize", Theme.CornerSize);
	Target->Markup.SetGlobal("Theme_TabAlign", UIBox::Align::Default);
	Target->Markup.SetGetStringFunction(&EditorUI::Asset);

	Target->Colors.ScrollBackgroundColor = Theme.Background;
	Target->Colors.ScrollBackgroundBorderColor = Theme.Background;
	Target->Colors.ScrollBarColor = Theme.DarkText;
	Target->Colors.TextFieldSelection = Theme.SelectedText;
	Target->Colors.TextFieldTextDefaultColor = Theme.Text;
	Target->Colors.KeyboardSelectionColor = Theme.Text;

	platform::SetWindowTheming(Theme.DarkBackground, Theme.Text, Theme.Highlight1,
		Theme.CornerSize.Value > 0, Target);

	if (Full && thread::IsMainThread && Instance)
	{
		Instance->Root->SetColor(Theme.DarkBackground);
		Instance->StatusBar->SetColor(Theme.DarkBackground);
		Instance->MenuBar->SetColor(Theme.DarkBackground);

		ForEachPanel<EditorPanel>([](EditorPanel* p) {
			p->OnThemeChanged();
		});

		Instance->RootPanel->ShouldUpdate = true;
		Instance->RootPanel->UpdatePanel();
	}
}

void engine::editor::EditorUI::Update()
{
	using namespace kui;

	if (DraggedBox)
	{
		if (CurrentDraggedItem.Centered)
			DraggedBox->SetPosition(DraggedBox->GetParentWindow()->Input.MousePosition
				- DraggedBox->GetUsedSize().GetScreen() / 2);
		else
			DraggedBox->SetPosition(DraggedBox->GetParentWindow()->Input.MousePosition
				- Vec2f(0, DraggedBox->GetUsedSize().GetScreen().Y));
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
		2 - UISize::Pixels(StatusBarSize + MenuBarSize).GetScreen().Y));
	MainBackground->GetParent()->UpdateElement();
	MainBackground->GetParent()->UpdateElement();
}

void engine::editor::EditorUI::AddMenuBarItem(string Name, std::vector<DropdownMenu::Option> Options)
{
	auto* btn = new MenuBarButton();
	btn->SetName(Name);

	btn->button->OnClicked = [btn, Options]()
	{
		new DropdownMenu(Options, btn->GetPosition());
	};
	MenuBar->AddChild(btn);
}

engine::string engine::editor::EditorUI::Asset(const string& Path)
{
	return GetEditorPath() + "/Editor/Assets/" + Path;
}
