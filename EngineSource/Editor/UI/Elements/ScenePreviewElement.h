#pragma once
#include <kui/UI/UIBackground.h>
#include <Engine/Scene.h>

namespace engine::editor
{
	class ScenePreviewElement : public kui::UIBackground
	{
	public:
		ScenePreviewElement(Scene* PreviewScene, kui::SizeVec Size);

		void Tick() override;
		void Update() override;

		void RedrawScene();

		Scene* PreviewScene = nullptr;
		bool RedrawNextFrame = false;
		bool AlwaysRedraw = false;
	};
}