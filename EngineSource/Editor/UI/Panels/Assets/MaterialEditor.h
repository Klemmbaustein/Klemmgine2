#ifdef EDITOR
#pragma once
#include <Engine/File/AssetRef.h>
#include <Engine/Scene.h>
#include <Engine/Graphics/Material.h>
#include <kui/UI/UIScrollBox.h>
#include "AssetEditor.h"

namespace engine::editor
{
	class MaterialEditor : public AssetEditor
	{
	public:

		kui::UIScrollBox* MaterialParamsBox = nullptr;

		MaterialEditor(AssetRef MaterialFile);
		~MaterialEditor() override;

		void Update();
		void LoadUI();
		void Save() override;

		void OnResized() override;

	private:

		void OnChanged() override;

		kui::UIBackground* PreviewImage = nullptr;
		Scene* PreviewScene = nullptr;
		graphics::Material* LoadedMaterial = nullptr;
	};
}
#endif