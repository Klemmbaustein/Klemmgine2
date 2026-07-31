#include "ModelAssetType.h"
#include <Editor/UI/Panels/Viewport.h>
#include <Editor/UI/Panels/Assets/ModelEditor.h>
#include <Engine/File/ModelData.h>
using namespace engine;

void engine::editor::ModelAssetType::ReloadAsset(AssetRef Asset)
{
	Log::Info("Reloading model file: " + Asset.FilePath);
	GraphicsModel::ReloadModel(Asset);
}

void engine::editor::ModelAssetType::Open(EditorUI* With, AssetRef Asset)
{
	Viewport::Current->AddChild(new ModelEditor(Asset), EditorPanel::Align::Tabs, true);
}

std::vector<string> engine::editor::ModelAssetType::GetExtensions() const
{
	return { "kmdl" };
}
