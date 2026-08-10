#include "ShaderAssetType.h"
#include <Engine/Graphics/ShaderLoader.h>
#include <Engine/File/Resource.h>
#include <Editor/UI/Panels/Assets/ShaderEditor.h>
#include <Editor/UI/Panels/Viewport.h>

using namespace engine;
using namespace engine::graphics;

void engine::editor::ShaderAssetType::ReloadAsset(AssetRef Asset)
{
	auto Found = ShaderLoader::Current->GetAllUsing(Asset.FilePath);

	for (auto& i : Found)
	{
		i.Object->ReCompile(resource::GetTextFile(i.VertexSource), resource::GetTextFile(i.FragmentSource));
	}
}

void engine::editor::ShaderAssetType::Open(EditorUI* With, AssetRef Asset)
{
	Viewport::Current->AddChild(new ShaderEditor(Asset), EditorPanel::Align::Tabs, true);
}

std::vector<string> engine::editor::ShaderAssetType::GetExtensions() const
{
	return { "vert", "frag" };
}
