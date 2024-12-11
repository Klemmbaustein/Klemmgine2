#include "ModelEditor.h"
#include <Engine/MainThread.h>
#include <Engine/Editor/UI/EditorUI.h>
#include <kui/UI/UISpinner.h>

using namespace kui;

engine::editor::ModelEditor::ModelEditor(AssetRef ModelFile)
	: EditorPanel("Model: " + ModelFile.DisplayName(), "")
{
	EditedModel = ModelFile;
	EditorScene = new Scene();
	EditorScene->BufferSize = 200;
	EditorScene->OnResized(EditorScene->BufferSize);

	CurrentObj = EditorScene->CreateObject<MeshObject>();

	SceneBackground = new UIBackground(false, 0, 1, 200);

	Background->AddChild(SceneBackground
		->SetPadding(5)
		->SetPaddingSizeMode(UIBox::SizeMode::PixelRelative)
		->SetSizeMode(UIBox::SizeMode::PixelRelative)
		->SetHorizontalAlign(UIBox::Align::Centered)
		->SetVerticalAlign(UIBox::Align::Centered)
		->AddChild(new UISpinner(0, EditorUI::Theme.Highlight1)));

	LoadModelThread = std::thread([this]()
		{
			GraphicsModel::RegisterModel(EditedModel);
			thread::ExecuteOnMainThread([this]()
				{
					CurrentObj->LoadMesh(EditedModel);
					SceneBackground->RedrawElement();
					ModelLoaded = true;
				});
		});
}

engine::editor::ModelEditor::~ModelEditor()
{
	LoadModelThread.join();
	GraphicsModel::UnloadModel(EditedModel);
	delete EditorScene;
}

void engine::editor::ModelEditor::Update()
{
	if (ModelLoaded)
	{
		SceneBackground->DeleteChildren();
	}
	SceneBackground->SetUseTexture(true, EditorScene->Buffer->Textures[0]);
}
