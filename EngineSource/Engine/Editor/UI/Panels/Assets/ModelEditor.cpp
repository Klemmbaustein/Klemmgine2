#include "ModelEditor.h"
#include <Engine/MainThread.h>
#include <Engine/Editor/UI/EditorUI.h>
#include <Engine/Editor/UI/Elements/AssetSelector.h>
#include <kui/UI/UISpinner.h>
#include <kui/Window.h>
using namespace kui;

engine::editor::ModelEditor::ModelEditor(AssetRef ModelFile)
	: EditorPanel("Model: " + ModelFile.DisplayName(), "")
{
	auto CancelLoadShared = std::make_shared<bool>(false);
	CancelLoad = CancelLoadShared;

	EditedModel = ModelFile;
	EditorScene = new Scene();
	EditorScene->BufferSize = 250 / Background->GetParentWindow()->GetDPI();
	EditorScene->Cam->Rotation = Vector3(-0.7f, -2.4f, 0);
	EditorScene->Cam->Position = Vector3(3, 3, 3);
	EditorScene->OnResized(EditorScene->BufferSize);

	CurrentObj = EditorScene->CreateObject<MeshObject>();

	Background->SetHorizontal(true);

	SceneBackground = new UIBackground(false, 0, 1, 250);

	SidebarBox = new UIScrollBox(false, 0, true);

	Background->AddChild(SidebarBox
		->SetPadding(5)
		->SetPaddingSizeMode(UIBox::SizeMode::PixelRelative)
	);

	SidebarBox->AddChild(SceneBackground
		->SetSizeMode(UIBox::SizeMode::PixelRelative)
		->SetHorizontalAlign(UIBox::Align::Centered)
		->SetVerticalAlign(UIBox::Align::Centered)
		->AddChild(new UISpinner(0, EditorUI::Theme.Highlight1)));

	SidebarBox->AddChild((new UIButton(true, 0, EditorUI::Theme.Text, [this]()
		{
			Save();
		}))
		->AddChild((new UIText(15, EditorUI::Theme.HighlightText, "save", EditorUI::EditorFont))
		->SetTextSizeMode(UIBox::SizeMode::PixelRelative)));

	MainBox = new UIScrollBox(false, 0, true);

	Background->AddChild(MainBox
		->SetPadding(5)
		->SetPaddingSizeMode(UIBox::SizeMode::PixelRelative)
		->SetTryFill(true));

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

void engine::editor::ModelEditor::OnModelLoaded()
{
	MainBox->DeleteChildren();

	GraphicsModel* Data = CurrentObj->DrawnModel;

	for (ModelData::Mesh& m : Data->Data->Meshes)
	{
		auto* NewSelector = new AssetSelector(AssetRef::FromPath(m.Material), 0.3f, nullptr);
		NewSelector->SelectedAsset.Extension = "kmt";
		NewSelector->OnChanged = [this, &m, NewSelector]()
			{
				m.Material = NewSelector->SelectedAsset.FilePath;
				OnModelChanged();
			};

		NewSelector->SetPadding(10, 10, 20, 20);
		NewSelector->SetPaddingSizeMode(UIBox::SizeMode::PixelRelative);

		MainBox->AddChild(NewSelector);
	}
}

void engine::editor::ModelEditor::OnModelChanged()
{
	CurrentObj->LoadMesh(EditedModel);
	SceneBackground->RedrawElement();
}

void engine::editor::ModelEditor::Update()
{
	if (ModelLoaded && SceneBackground->GetChildren().size())
	{
		SceneBackground->DeleteChildren();
		OnModelLoaded();
	}
	if (EditorUI::FocusedPanel == this)
		SceneBackground->SetUseTexture(true, EditorScene->Buffer->Textures[0]);
}

void engine::editor::ModelEditor::Save()
{
	GraphicsModel* Data = CurrentObj->DrawnModel;
	Data->Data->ToFile(EditedModel.FilePath);
}
