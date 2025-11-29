#include "ModelEditor.h"
#include <Engine/MainThread.h>
#include <Editor/UI/EditorUI.h>
#include <Editor/UI/Elements/Checkbox.h>
#include <Editor/UI/Elements/AssetSelector.h>
#include <kui/UI/UISpinner.h>
#include <kui/Window.h>
#include <Toolbar.kui.hpp>
#include <Editor/UI/Windows/MessageWindow.h>
#include <Core/Log.h>
#include <Editor/UI/Elements/Toolbar.h>
#include <Engine/Input.h>
using namespace kui;

engine::editor::ModelEditor::ModelEditor(AssetRef ModelFile)
	: AssetEditor("Model: %s", ModelFile)
{
	auto CancelLoadShared = std::make_shared<bool>(false);
	CancelLoad = CancelLoadShared;

	EditorScene = new Scene();
	EditorScene->Physics.Active = false;
	EditorScene->Resizable = false;
	EditorScene->AlwaysRedraw = false;
	EditorScene->Redraw = true;

	CurrentObj = EditorScene->CreateObject<MeshObject>();

	Background->SetHorizontal(true);

	SceneBackground = new UIBackground(false, 0, 1, 0);
	SceneBackground->SetCorner(5_px);
	SceneBackground->SetCorners(false, false, true, false);

	MainBox = new UIBox(false);

	Background->AddChild(MainBox
		->SetPadding(1_px, 5_px, 1_px, 3_px));

	auto ModelToolbar = new Toolbar(false);

	ModelToolbar->AddButton("Save", "file:Engine/Editor/Assets/Save.png",
		[this] {
		Save();
	});

	MainBox->AddChild(ModelToolbar);

	MainBox->AddChild(SceneBackground
		->SetBorder(1_px, EditorUI::Theme.BackgroundHighlight)
		->SetBorderEdges(false, false, false, true)
		->SetHorizontalAlign(UIBox::Align::Centered)
		->SetVerticalAlign(UIBox::Align::Centered)
		->AddChild(new UISpinner(0, EditorUI::Theme.Highlight1)));

	Sidebar = new PropertyMenu();

	Background->AddChild(Sidebar
		->SetMinSize(SizeVec(290_px, UISize::Parent(1)))
		->SetMaxSize(SizeVec(290_px, UISize::Parent(1)))
		->SetPadding(8_px, 3_px, 0, 0));

	LoadModelThread = std::thread([this, ModelFile, CancelLoadShared] {
		auto Loaded = GraphicsModel::RegisterModel(ModelFile);

		if (!Loaded)
		{
			FailedLoading = true;
			return;
		}

		Loaded->Data->PreLoadMaterials(EditorScene);
		if (*CancelLoadShared)
		{
			GraphicsModel::UnloadModel(ModelFile);
			return;
		}
		thread::ExecuteOnMainThread([this, ModelFile, CancelLoadShared] {
			if (*CancelLoadShared)
			{
				GraphicsModel::UnloadModel(ModelFile);
				return;
			}
			this->CurrentObj->LoadMesh(ModelFile);

			Vector3 Pos = Vector3(0.75f) * CurrentObj->Mesh->DrawnModel->Data->Bounds.Extents.Length()
				+ CurrentObj->Mesh->DrawnModel->Data->Bounds.Position;

			EditorScene->SceneCamera->Position = Pos;
			EditorScene->SceneCamera->Rotation = Vector3(-35, 45, 0);
			this->SceneBackground->RedrawElement();
			this->ModelLoaded = true;
			this->EditorScene->Redraw = true;
		});
	});
}

engine::editor::ModelEditor::~ModelEditor()
{
	*CancelLoad = true;
	LoadModelThread.detach();
	if (ModelLoaded)
		GraphicsModel::UnloadModel(EditedAsset);
	delete EditorScene;
}

void engine::editor::ModelEditor::OnModelLoaded()
{
	Sidebar->Clear();
	GraphicsModel* Data = CurrentObj->Mesh->DrawnModel;

	if (!Data)
	{
		Sidebar->SetMode(PropertyMenu::Mode::DisplayText);
		if (!FailedLoading)
		{
			Sidebar->AddChild((new UISpinner(0, EditorUI::Theme.Highlight1, 30_px))
				->SetBackgroundColor(EditorUI::Theme.LightBackground)
				->SetPadding(10_px));
		}
		else
		{
			SceneBackground->AddChild((new UIBox(true, 0))
				->SetVerticalAlign(UIBox::Align::Centered)
				->AddChild((new UIBackground(true, 0, 1, 24_px))
					->SetUseTexture(true, "Engine/Editor/Assets/Error.png")
					->SetPadding(10_px))
				->AddChild(new UIText(11_px, 1, "Model failed to load.\nCheck the log for details", EditorUI::EditorFont)));
		}
		return;
	}

	Sidebar->SetMode(PropertyMenu::Mode::DisplayEntries);
	Sidebar->CreateNewHeading("Materials");

	size_t it = 0;
	for (ModelData::Mesh& m : Data->Data->Meshes)
	{
		auto NewEntry = Sidebar->CreateNewEntry(std::to_string(it++));
		auto* NewSelector = new AssetSelector(AssetRef::FromName(m.Material, "kmt"), Sidebar->ElementSize, nullptr);
		NewSelector->SelectedAsset.Extension = "kmt";
		NewSelector->OnChanged = [this, &m, NewSelector]()
		{
			auto NewMaterial = NewSelector->SelectedAsset.DisplayName();

			if (NewMaterial == m.Material)
				return;

			m.Material = NewMaterial;
			OnChanged();
			OnModelChanged();
		};

		NewEntry->valueBox->AddChild(NewSelector);
	}

	UIBox* PropertiesBox = new UIBox(false);

	Sidebar->CreateNewHeading("Properties");

	auto ModelChanged = [this] {
		OnChanged();
		OnModelChanged();
	};

	size_t Vertices = 0;
	size_t Indices = 0;

	for (auto& i : Data->Data->Meshes)
	{
		Vertices += i.Vertices.size();
		Indices += i.Indices.size();
	}

	Sidebar->AddBooleanEntry("Collision", Data->Data->HasCollision, ModelChanged);
	Sidebar->AddBooleanEntry("Shadow", Data->Data->CastShadow, ModelChanged);

	Sidebar->CreateNewHeading("Statistics");
	Sidebar->AddInfoEntry("Vertices", std::to_string(Vertices));
	Sidebar->AddInfoEntry("Triangles", std::to_string(Indices / 3));
	Sidebar->AddInfoEntry("Center", Data->Data->Bounds.Position.ToString());
	Sidebar->AddInfoEntry("Extents", Data->Data->Bounds.Extents.ToString());
	Sidebar->AddInfoEntry("RefCount", std::to_string(Data->References));
}

void engine::editor::ModelEditor::OnModelChanged()
{
	CurrentObj->LoadMesh(EditedAsset);
	EditorScene->Redraw = true;
	SceneBackground->RedrawElement();
}
void engine::editor::ModelEditor::Update()
{
	AssetEditor::Update();
	if ((ModelLoaded || FailedLoading) && SceneBackground->GetChildren().size())
	{
		SceneBackground->DeleteChildren();

		OnModelLoaded();
	}
	if (EditorUI::FocusedPanel == this)
		SceneBackground->SetUseImage(true, EditorScene->GetDrawBuffer());
}

void engine::editor::ModelEditor::Save()
{
	AssetEditor::Save();
	GraphicsModel* Data = CurrentObj->Mesh->DrawnModel;
	Log::Info("Writing model data to file: " + EditedAsset.FilePath);

	auto Path = EditorUI::Instance->AssetsProvider->GetFileSaveStream(EditedAsset.FilePath);

	if (Path)
	{
		Data->Data->ToBinary(Path);
		delete Path;
	}
	else
	{
		BufferStream Buffer;
		Data->Data->ToBinary(&Buffer);
		Buffer.ResetStreamPosition();
		EditorUI::Instance->AssetsProvider->SaveToFile(EditedAsset.FilePath, &Buffer, Buffer.GetSize());
	}
}

void engine::editor::ModelEditor::OnResized()
{
	Vec2i PixelSize = Vec2f(Size
		* Vec2f(Background->GetParentWindow()->GetSize())
		/ Vec2f(Background->GetParentWindow()->GetDPI())
		/ Vec2f(2.0f));

	PixelSize = Vec2i::Max(PixelSize - Vec2i(300, 42), Vec2i(10));

	EditorScene->BufferSize = PixelSize * Background->GetParentWindow()->GetDPI();
	EditorScene->OnResized(EditorScene->BufferSize);
	SceneBackground->SetMinSize(SizeVec(PixelSize, SizeMode::PixelRelative));
	SceneBackground->SetMaxSize(SizeVec(PixelSize, SizeMode::PixelRelative));
	this->EditorScene->Redraw = true;
	OnModelLoaded();
}
