#pragma once
#include <Engine/Editor/UI/Panels/EditorPanel.h>
#include <Engine/File/AssetRef.h>
#include <Engine/Scene.h>
#include <thread>
#include <Engine/Objects/MeshObject.h>

namespace engine::editor
{
	class ModelEditor : public EditorPanel
	{
	public:
		ModelEditor(AssetRef ModelFile);
		~ModelEditor();

		void Update();

		bool ModelLoaded = false;
		AssetRef EditedModel;
		kui::UIBackground* SceneBackground = nullptr;
		Scene* EditorScene = nullptr;
		MeshObject* CurrentObj = nullptr;
		std::thread LoadModelThread;
	};
}