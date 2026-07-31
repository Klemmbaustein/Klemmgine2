#pragma once
#include <Editor/Assets/EditorAssetType.h>

namespace engine::editor
{
	class ModelAssetType : public EditorAssetType
	{
	public:

		// Inherited via EditorAssetType
		void ReloadAsset(AssetRef Asset) override;
		void Open(EditorUI* With, AssetRef Asset) override;
		std::vector<string> GetExtensions() const override;
	};
}