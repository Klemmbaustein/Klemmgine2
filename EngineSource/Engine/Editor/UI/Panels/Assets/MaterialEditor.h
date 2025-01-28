#ifdef EDITOR
#pragma once
#include <Engine/Editor/UI/Panels/EditorPanel.h>
#include <Engine/File/AssetRef.h>
#include <Engine/Scene.h>
#include <Engine/Graphics/Material.h>
#include <kui/UI/UIScrollBox.h>

namespace engine::editor
{
	class MaterialEditor : public EditorPanel
	{
	public:

		kui::UIScrollBox* MaterialParamsBox = nullptr;

		MaterialEditor(AssetRef MaterialFile);
		~MaterialEditor() override;

		void Update();
		void LoadUI();

	private:

		kui::UIBackground* PreviewImage = nullptr;
		Scene* PreviewScene = nullptr;
		graphics::Material* LoadedMaterial = nullptr;
	};
}
#endif