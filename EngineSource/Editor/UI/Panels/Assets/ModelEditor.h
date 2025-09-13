#ifdef EDITOR
#pragma once
#include "AssetEditor.h"
#include <Engine/File/AssetRef.h>
#include <Engine/Scene.h>
#include <thread>
#include <Engine/Objects/MeshObject.h>
#include <memory>

namespace engine::editor
{
	class ModelEditor : public AssetEditor
	{
	public:
		ModelEditor(AssetRef ModelFile);
		~ModelEditor();

		void Update();

		void Save() override;

		void OnResized() override;

	private:
		void OnModelLoaded();

		void OnModelChanged();

		bool ModelLoaded = false;
		kui::UIBackground* SceneBackground = nullptr;
		kui::UIBox* SidebarBox = nullptr;
		kui::UIBox* MainBox = nullptr;
		Scene* EditorScene = nullptr;
		MeshObject* CurrentObj = nullptr;
		std::thread LoadModelThread;
		std::shared_ptr<bool> CancelLoad;
		bool FailedLoading = false;
	};
}
#endif