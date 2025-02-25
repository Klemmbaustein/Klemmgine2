#ifdef EDITOR
#pragma once
#include <Engine/File/AssetRef.h>
#include <Editor/UI/Panels/EditorPanel.h>

namespace engine::editor
{
	class AssetEditor : public EditorPanel
	{
	public:

		AssetEditor(string NameFormat, AssetRef Asset);

		virtual void OnChanged();
		virtual void Save();

		virtual void Update() override;

		virtual bool OnClosed() override;

		engine::AssetRef EditedAsset;

	private:
		string NameFormat;
		bool Saved = true;
		void UpdateName();
	};
}
#endif