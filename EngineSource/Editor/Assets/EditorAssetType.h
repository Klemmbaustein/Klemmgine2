#pragma once
#include <Core/Types.h>
#include <Engine/File/AssetRef.h>

namespace engine::editor
{
	class EditorUI;

	class EditorAssetType
	{
	public:
		virtual ~EditorAssetType() = default;

		virtual void ReloadAsset(AssetRef Asset) = 0;
		virtual void Open(EditorUI* With, AssetRef Asset) = 0;

		virtual std::vector<string> GetExtensions() const = 0;
	};
}