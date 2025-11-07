#ifdef EDITOR
#pragma once
#include <Engine/File/AssetRef.h>
#include <Engine/Scene.h>
#include <Engine/Graphics/Material.h>
#include <kui/UI/UIScrollBox.h>
#include "AssetEditor.h"
#include <iostream>
#include <Editor/UI/Elements/PropertyMenu.h>

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

		PropertyMenu* MaterialSettings = nullptr;

		void CreateTextureField(kui::UIBox* Parent, graphics::Material::Field& Field);

		bool RedrawNextFrame = false;
		kui::UIBackground* Sidebar = nullptr;
		kui::UIScrollBox* SidebarScroll = nullptr;
		kui::UIBackground* PreviewImage = nullptr;
		kui::UIBox* MainBox = nullptr;
		Scene* PreviewScene = nullptr;
		graphics::Material* LoadedMaterial = nullptr;
	};
}
#endif