#include "ProjectSettingsWindow.h"
#include <Engine/MainThread.h>
#include <Editor/UI/Elements/PropertyMenu.h>
#include <Editor/EditorSubsystem.h>
#include <Engine/Engine.h>

using namespace kui;
using namespace engine::editor;

engine::editor::ProjectSettingsWindow::ProjectSettingsWindow()
	: IDialogWindow("Project settings", { IDialogWindow::Option{
		.Name = "Ok", .IsAccept = true, .IsClose = true, } }, Vec2ui(400, 200))
{
	auto Editor = Engine::Instance->GetSubsystem<EditorSubsystem>();

	Name = Editor->Project.Name;
	StartupScene = Editor->Project.StartupScene.empty()
		? AssetRef::FromPath(Editor->Project.StartupScene) : StartupScene;

	if (!StartupScene.Exists())
	{
		StartupScene = AssetRef::EmptyAsset("kts");
	}

	this->Open();
}

void engine::editor::ProjectSettingsWindow::Begin()
{
	IDialogWindow::Begin();

	auto ProjectSettings = new PropertyMenu();
	ProjectSettings->SetSize(SizeVec(UISize::Pixels(350), UISize::Parent(1)));
	this->Background->AddChild(ProjectSettings);

	ProjectSettings->CreateNewHeading("Project settings");
	ProjectSettings->AddStringEntry("Name", this->Name, nullptr);
	ProjectSettings->AddAssetRefEntry("Startup scene", this->StartupScene, nullptr);

	this->Background->SetHorizontalAlign(UIBox::Align::Centered);
}

void engine::editor::ProjectSettingsWindow::Update()
{
}

void engine::editor::ProjectSettingsWindow::Destroy()
{
	thread::ExecuteOnMainThread([Name = this->Name, StartupScene = this->StartupScene]() {
		auto Editor = Engine::Instance->GetSubsystem<EditorSubsystem>();

		Editor->Project.Name = Name.empty() ? "Untitled" : Name;
		Editor->Project.StartupScene = StartupScene.FilePath;
		Editor->Project.Save("Project.json");
	});
}
