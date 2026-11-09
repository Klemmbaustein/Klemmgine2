#pragma once
#include <Editor/UI/Panels/Assets/AssetEditor.h>
#include <Editor/UI/Elements/ScenePreviewElement.h>
#include <Engine/Objects/ParticleObject.h>
#include <Engine/Scene.h>
#include <Engine/Debug/DebugDraw.h>
#include <Editor/UI/Elements/PropertyMenu.h>

namespace engine::editor
{
	class ParticleEditor : public AssetEditor
	{
	public:

		ParticleEditor(AssetRef ParticleFile);
		~ParticleEditor();

		void OnResized() override;
		void OnThemeChanged() override;
		void Update() override;

		void Save() override;

		void BuildLayout();
		void UpdateSideBar();

	private:
		Scene* PreviewScene = nullptr;
		ScenePreviewElement* PreviewElement = nullptr;
		ParticleObject* PreviewObject = nullptr;
		kui::UIScrollBox* SideBarBox = nullptr;
		kui::UIScrollBox* ContentBox = nullptr;
		kui::UIBackground* SideBarBackground = nullptr;
		debug::DebugBox* BoundsShape = nullptr;
		kui::UIBox* MainBox = nullptr;
		PropertyMenu* SideBarMenu = nullptr;

		bool ShowBounds = false;
		bool IsBoundsShapeAdded = false;
		bool AutoRepeat = true;
	};
}