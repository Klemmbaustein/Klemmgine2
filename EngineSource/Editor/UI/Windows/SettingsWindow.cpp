#include "SettingsWindow.h"
#include <Editor/UI/Settings/GraphicsSettingsPage.h>
#include <Editor/UI/Settings/InterfaceSettingsPage.h>
#include <Editor/UI/Elements/PropertyMenu.h>
#include <kui/UI/UIButton.h>
#include <kui/UI/UIText.h>
#include <Editor/UI/EditorUI.h>

using namespace kui;

engine::editor::SettingsWindow::SettingsWindow()
	: IDialogWindow("Editor Settings", {
		Option{.Name = "Close", .Close = true, } }, Vec2ui(640, 480))
{
	this->Open();
}

engine::editor::SettingsWindow::~SettingsWindow()
{
}

void engine::editor::SettingsWindow::Begin()
{
	IDialogWindow::Begin();

	Pages.push_back(new InterfaceSettingsPage());
	Pages.push_back(new GraphicsSettingsPage());

	Background->SetHorizontal(true);

	Sidebar = new UIScrollBox(false, 0, true);
	Sidebar->SetMinSize(SizeVec(160_px, UISize::Parent(1)));
	Sidebar->SetMaxSize(SizeVec(160_px, UISize::Parent(1)));
	SettingsMenu = new PropertyMenu();
	SettingsMenu->SetMinSize(SizeVec(380_px, UISize::Parent(1)));
	SettingsMenu->SetMaxSize(SizeVec(380_px, UISize::Parent(1)));
	SettingsMenu->SetLeftPadding(50_px);
	this->Background->AddChild((new UIBox(true, 0))
		->SetMinSize(UISize::Parent(1))
		->AddChild(Sidebar)
		->AddChild(SettingsMenu));

	ShowPage(Pages.at(0));
}

void engine::editor::SettingsWindow::Update()
{
}

void engine::editor::SettingsWindow::Destroy()
{
	for (auto& i : Pages)
	{
		delete i;
	}
}

void engine::editor::SettingsWindow::GenerateTabs()
{
	Sidebar->DeleteChildren();

	for (auto& i : Pages)
	{
		Vec3f BgColor = ActivePage == i ? EditorUI::Theme.HighlightDark : EditorUI::Theme.Background;
		Vec3f OutlineColor = ActivePage == i ? EditorUI::Theme.Highlight1 : EditorUI::Theme.Background;

		Sidebar->AddChild((new UIButton(true, 0, BgColor, [i, this]() {
			ShowPage(i);
		}))
			->SetBorder(1_px, OutlineColor)
			->SetCorner(5_px)
			->SetVerticalAlign(UIBox::Align::Centered)
			->SetMinWidth(UISize::Parent(1))
			->SetPadding(5_px, 0, 5_px, 5_px)
			->AddChild((new UIText(12_px, EditorUI::Theme.Text, i->Name, EditorUI::EditorFont))
				->SetPadding(4_px)));
	}
}

void engine::editor::SettingsWindow::ShowPage(SettingsPage* Page)
{
	ActivePage = Page;
	GenerateTabs();
	SettingsMenu->Clear();
	Page->Generate(SettingsMenu);
}
