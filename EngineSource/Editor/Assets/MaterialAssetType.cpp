#include "MaterialAssetType.h"
#include <Editor/UI/Panels/Viewport.h>
#include <Editor/UI/Panels/Assets/MaterialEditor.h>
#include <Engine/File/Resource.h>

using namespace engine;

void engine::editor::MaterialAssetType::ReloadAsset(AssetRef Asset)
{
}

void engine::editor::MaterialAssetType::Open(EditorUI* With, AssetRef Asset)
{
	Viewport::Current->AddChild(new MaterialEditor(Asset), EditorPanel::Align::Tabs, true);
}

std::vector<string> engine::editor::MaterialAssetType::GetExtensions() const
{
	return { "kmt", "kbm" };
}
