#pragma once
#include <Engine/File/AssetRef.h>
#include <Engine/Scene.h>
#include <Engine/Graphics/Material.h>
#include <kui/UI/UIScrollBox.h>
#include "AssetEditor.h"
#include <Editor/UI/Elements/ScenePreviewElement.h>
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

		const string DEFAULT_VERTEX_SHADER = "res:shader/basic.vert";
		const string DEFAULT_FRAGMENT_SHADER = "res:shader/basic.frag";

		AssetRef VertexShader = AssetRef::EmptyAsset("vert");
		AssetRef FragmentShader = AssetRef::EmptyAsset("frag");

		void OnChanged() override;

		PropertyMenu* MaterialSettings = nullptr;

		void CreateTextureField(kui::UIBox* Parent, graphics::Material::Field& Field);

		kui::UIBackground* Sidebar = nullptr;
		kui::UIScrollBox* SidebarScroll = nullptr;
		kui::UIBox* MainBox = nullptr;
		ScenePreviewElement* PreviewImage = nullptr;
		Scene* PreviewScene = nullptr;
		graphics::Material* LoadedMaterial = nullptr;
	};
}