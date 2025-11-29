#include "AboutWindow.h"
#include <Editor/UI/Elements/PropertyMenu.h>
#include <Engine/Version.h>
#include <Core/Platform/Platform.h>

using namespace kui;

engine::editor::AboutWindow::AboutWindow()
	: IDialogWindow("About Klemmgine 2", { Option{.Name = "Ok", .Close = true} }, Vec2ui(400, 200))
{
	this->Open();
}

void engine::editor::AboutWindow::Begin()
{
	IDialogWindow::Begin();

	auto m = new PropertyMenu(this->DefaultFont);
	m->SetMinWidth(UISize::Parent(1));
	this->Background->AddChild(m);

	VersionInfo Version = VersionInfo::Get();

	m->CreateNewHeading(Version.GetShortName());
	m->AddInfoEntry("Build type", Version.GetConfiguration());
	m->AddInfoEntry("Build ID", Version.Build);
	m->AddButtonsEntry("Attribution", { {"Github", [] {
		platform::Open("https://github.com/Klemmbaustein/Klemmgine2");
	}}, {"Licenses", [] {
		platform::Open("https://github.com/Klemmbaustein/Klemmgine2/blob/main/LICENSE.txt");
	}} });
}

void engine::editor::AboutWindow::Update()
{
}

void engine::editor::AboutWindow::Destroy()
{
}
