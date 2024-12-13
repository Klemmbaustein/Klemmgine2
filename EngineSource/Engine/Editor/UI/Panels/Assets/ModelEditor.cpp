#include "ModelEditor.h"
#include <Engine/MainThread.h>
#include <Engine/Editor/UI/EditorUI.h>
#include <kui/UI/UISpinner.h>
using namespace kui;

engine::editor::ModelEditor::ModelEditor(AssetRef ModelFile)
	: EditorPanel("Model: " + ModelFile.DisplayName(), "")
{
	auto CancelLoadShared = std::make_shared<bool>(false);
	CancelLoad = CancelLoadShared;

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

	LoadModelThread = std::thread([this, ModelFile, CancelLoadShared]()
		{
			GraphicsModel::RegisterModel(ModelFile);
			if (*CancelLoadShared)
			{
				GraphicsModel::UnloadModel(ModelFile);
				return;
			}
			thread::ExecuteOnMainThread([this, ModelFile, CancelLoadShared]()
				{
					if (*CancelLoadShared)
					{
						GraphicsModel::UnloadModel(ModelFile);
						return;
					}
					this->CurrentObj->LoadMesh(ModelFile);
					this->SceneBackground->RedrawElement();
					this->ModelLoaded = true;
				});
		});
}

engine::editor::ModelEditor::~ModelEditor()
{
	*CancelLoad = true;
	LoadModelThread.detach();
	if (ModelLoaded)
		GraphicsModel::UnloadModel(EditedModel);
	delete EditorScene;
}

void engine::editor::ModelEditor::Update()
{
	if (ModelLoaded)
	{
		SceneBackground->DeleteChildren();
	}
	if (EditorUI::FocusedPanel == this)
		SceneBackground->SetUseTexture(true, EditorScene->Buffer->Textures[0]);
}
