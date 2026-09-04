#pragma once
#include <Editor/Assets/EditorAssetType.h>
#include <Editor/UI/Elements/ScriptEditorUI.h>

namespace engine::editor
{
	class ScriptAssetType : public EditorAssetType
	{
	public:

		// Inherited via EditorAssetType
		void ReloadAsset(AssetRef Asset) override;
		void Open(EditorUI* With, AssetRef Asset) override;
		std::vector<string> GetExtensions() const override;

		void RunOnActiveScriptEditor(std::function<void(ScriptEditorUI*)> Function);
	};
}