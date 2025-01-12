#ifdef EDITOR
#include "ModelEditor.h"
#include <Engine/MainThread.h>
#include <Engine/Editor/UI/EditorUI.h>
#include <Engine/Editor/UI/Elements/AssetSelector.h>
#include <kui/UI/UISpinner.h>
#include <kui/Window.h>
#include <Toolbar.kui.hpp>
#include <Engine/Log.h>
using namespace kui;

engine::editor::ModelEditor::ModelEditor(AssetRef ModelFile)
	: EditorPanel("Model: " + ModelFile.DisplayName(), "")
{
	auto CancelLoadShared = std::make_shared<bool>(false);
	CancelLoad = CancelLoadShared;

	EditedModel = ModelFile;
	EditorScene = new Scene();
	EditorScene->Physics.Active = false;
	EditorScene->SceneCamera->Rotation = Vector3(-0.7f, -2.4f, 0);
	EditorScene->SceneCamera->Position = Vector3(3, 3, 3);
	EditorScene->Resizable = false;

	CurrentObj = EditorScene->CreateObject<MeshObject>();

	Background->SetHorizontal(true);

	SceneBackground = new UIBackground(false, 0, 1, 0);
	SceneBackground->SetCorner(5_px);
	SceneBackground->SetCorners(false, false, true, false);

	MainBox = new UIScrollBox(false, 0, true);

	Background->AddChild(MainBox
		->SetPadding(0_px, 5_px, 1_px, 5_px));

	auto SaveButton = new ToolBarButton();
	SaveButton->SetIcon("Engine/Editor/Assets/Save.png");
	SaveButton->SetName("Save");
	SaveButton->btn->OnClicked = [this]() {
		Save();
		};
	delete SaveButton->dropdownButton;

	MainBox->AddChild((new UIBox(true))
		->SetMinSize(SizeVec(UISize::Parent(1), 38_px))
		->AddChild(SaveButton));

	MainBox->AddChild(SceneBackground
		->SetBorder(1_px, EditorUI::Theme.BackgroundHighlight)
		->SetHorizontalAlign(UIBox::Align::Centered)
		->SetVerticalAlign(UIBox::Align::Centered)
		->AddChild(new UISpinner(0, EditorUI::Theme.Highlight1)));

	SidebarBox = new UIScrollBox(false, 0, true);

	Background->AddChild(SidebarBox
		->SetMinSize(SizeVec(280_px, UISize::Parent(1)))
		->SetHorizontalAlign(UIBox::Align::Centered)
		->SetPadding(5_px));

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
	SidebarBox->DeleteChildren();

	SidebarBox->AddChild((new UIBox(true))
		->SetMinWidth(UISize::Parent(1))
		->AddChild(new UIText(14_px, 1, "Materials", EditorUI::EditorFont)));

	GraphicsModel* Data = CurrentObj->Mesh->DrawnModel;

	if (!Data)
	{
		SidebarBox->AddChild((new UISpinner(0, EditorUI::Theme.Highlight1, 30))
			->SetBackgroundColor(EditorUI::Theme.HighlightDark));
		return;
	}
	float Width = UIBox::PixelSizeToScreenSize(245, Background->GetParentWindow()).X;

	for (ModelData::Mesh& m : Data->Data->Meshes)
	{
		auto* NewSelector = new AssetSelector(AssetRef::FromPath(m.Material), Width, nullptr);
		NewSelector->SelectedAsset.Extension = "kmt";
		NewSelector->OnChanged = [this, &m, NewSelector]()
			{
				m.Material = NewSelector->SelectedAsset.FilePath;
				OnModelChanged();
			};

		NewSelector->SetPadding(10_px, 10_px, 20_px, 20_px);

		SidebarBox->AddChild(NewSelector);
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
	GraphicsModel* Data = CurrentObj->Mesh->DrawnModel;
	Data->Data->ToFile(EditedModel.FilePath);
}

void engine::editor::ModelEditor::OnResized()
{
	Vec2i PixelSize = Vec2f(Size
		* Vec2f(Background->GetParentWindow()->GetSize())
		/ Vec2f(Background->GetParentWindow()->GetDPI())
		/ Vec2f(2.0f));

	PixelSize = Vec2i::Max(PixelSize - Vec2i(300, 39), Vec2i(10));

	EditorScene->BufferSize = PixelSize * Background->GetParentWindow()->GetDPI();
	EditorScene->OnResized(EditorScene->BufferSize);
	SceneBackground->SetMinSize(SizeVec(PixelSize));
	SceneBackground->SetMaxSize(SizeVec(PixelSize));
	OnModelLoaded();
}
#endif