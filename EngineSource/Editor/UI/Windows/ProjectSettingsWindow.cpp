#include "ProjectSettingsWindow.h"
#include <Engine/MainThread.h>
#include <Editor/UI/Elements/PropertyMenu.h>
#include <Editor/EditorSubsystem.h>
#include <Engine/Engine.h>
#include <Engine/Script/ScriptSubsystem.h>

using namespace kui;
using namespace engine::editor;

engine::editor::ProjectSettingsWindow::ProjectSettingsWindow()
	: IDialogWindow("Project settings", { IDialogWindow::Option{
		.Name = "Ok", .IsAccept = true, .IsClose = true, } }, Vec2ui(400, 300))
{
	auto Engine = Engine::Instance;

	Name = Engine->OpenedProject->Name;
	StartupScene = !Engine->OpenedProject->StartupScene.empty()
		? AssetRef::FromPath(Engine->OpenedProject->StartupScene) : StartupScene;

	this->UseScriptJIT = Engine->OpenedProject->UseScriptJIT;

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
	ProjectSettings->AddBooleanEntry("Use Script JIT", this->UseScriptJIT, nullptr);

	this->Background->SetHorizontalAlign(UIBox::Align::Centered);
}

void engine::editor::ProjectSettingsWindow::Update()
{
}

void engine::editor::ProjectSettingsWindow::Destroy()
{
	thread::ExecuteOnMainThread([Name = this->Name, StartupScene = this->StartupScene, UseScriptJIT = this->UseScriptJIT]() {
		auto Engine = Engine::Instance;

		Engine->OpenedProject->Name = Name.empty() ? "Untitled" : Name;
		Engine->OpenedProject->StartupScene = StartupScene.FilePath;
		Engine->OpenedProject->UseScriptJIT = UseScriptJIT;
		Engine->OpenedProject->Save("Project.json");
		script::ScriptSubsystem::Instance->ReloadRuntime();
	});
}
