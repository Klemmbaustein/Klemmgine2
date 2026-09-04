#include "ScriptAssetType.h"
#include <Engine/Engine.h>
#include <Editor/UI/EditorUI.h>
#include <Editor/UI/Panels/ClassBrowser.h>
#include <Engine/Script/ScriptSubsystem.h>
#include <Editor/Settings/EditorSettings.h>
#include <Core/Platform/Platform.h>
#include <Engine/File/Resource.h>
#include <filesystem>
#include <Editor/UI/Windows/ScriptEditorWindow.h>
#include <Editor/UI/Panels/ScriptEditorPanel.h>

using namespace engine;

void engine::editor::ScriptAssetType::ReloadAsset(AssetRef Asset)
{
	Engine::Instance->GetSubsystem<script::ScriptSubsystem>()->Reload();

	EditorUI::SetStatusMessage("Script files changed externally. Reloading scripts", EditorUI::StatusType::Info);

	EditorUI::ForEachPanel<ClassBrowser>([](ClassBrowser* Browser) {
		Browser->UpdateItems();
	});
}

void engine::editor::ScriptAssetType::Open(EditorUI* With, AssetRef Asset)
{
	auto& Setting = Settings::GetInstance()->Script;

	if (resource::AllowLocalFiles && Setting.GetSetting("useExternalEditor", false).GetBool())
	{
		if (Setting.GetSetting("useDefaultEditor", true).GetBool())
		{
			platform::Open(Asset.FilePath);
		}
		auto Name = Setting.GetSetting("externalEditorCommand", "").GetString();
		auto Arguments = Setting.GetSetting("externalEditorArguments", "").GetString();

		Arguments = str::Replace(Arguments, "{file}", std::filesystem::canonical(Asset.FilePath).string());
		Arguments = str::Replace(Arguments, "{workspace}",
			std::filesystem::canonical(std::filesystem::current_path()).string());

		platform::Execute(Name, " " + Arguments);

		return;
	}

	RunOnActiveScriptEditor([FilePath = Asset.FilePath](ScriptEditorUI* UI) {
		UI->NavigateTo(FilePath, {});
	});
}

std::vector<string> engine::editor::ScriptAssetType::GetExtensions() const
{
	return { "ds", "kui" };
}

void engine::editor::ScriptAssetType::RunOnActiveScriptEditor(std::function<void(ScriptEditorUI*)> Function)
{
	if (ScriptEditorWindow::Current)
	{
		ScriptEditorWindow::Current->Queue->Run([Function] {
			Function(ScriptEditorWindow::Current->UI);
		});
		return;
	}

	EditorUI::ForEachPanel<ScriptEditorPanel>([Function](ScriptEditorPanel* p) {
		Function(&p->UI);
		p->SetFocused();
	});
}
