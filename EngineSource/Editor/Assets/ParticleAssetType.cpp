#include "ParticleAssetType.h"
#include <Editor/UI/Panels/Viewport.h>
#include <Editor/UI/Panels/Assets/ParticleEditor.h>

using namespace engine;

void engine::editor::ParticleAssetType::ReloadAsset(AssetRef Asset)
{
}

void engine::editor::ParticleAssetType::Open(EditorUI* With, AssetRef Asset)
{
	Viewport::Current->AddChild(new ParticleEditor(Asset), EditorPanel::Align::Tabs, true);
}

std::vector<string> engine::editor::ParticleAssetType::GetExtensions() const
{
	return { "prt" };
}
