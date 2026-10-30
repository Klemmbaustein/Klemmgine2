#pragma once
#include <Editor/UI/Panels/Assets/AssetEditor.h>
#include <Editor/UI/Elements/ScenePreviewElement.h>
#include <Engine/Objects/ParticleObject.h>
#include <Engine/Scene.h>
#include <Engine/Debug/DebugDraw.h>

namespace engine::editor
{
	class ParticleEditor : public AssetEditor
	{
	public:

		ParticleEditor(AssetRef ParticleFile);
		~ParticleEditor();

		void OnResized() override;
		void Update() override;

		void BuildLayout();

	private:
		Scene* PreviewScene = nullptr;
		ScenePreviewElement* PreviewElement = nullptr;
		ParticleObject* PreviewObject = nullptr;
		kui::UIScrollBox* ContentBox = nullptr;
		debug::DebugBox* BoundsShape = nullptr;

		bool ShowBounds = false;
		bool IsBoundsShapeAdded = false;
	};
}