#include "ParticleEditor.h"
#include <ParticleEditor.kui.hpp>
#include <kui/UI/UIScrollBox.h>
#include <Editor/UI/Elements/Toolbar.h>
#include <sstream>
#include <Core/File/TextSerializer.h>

using namespace kui;

engine::editor::ParticleEditor::ParticleEditor(AssetRef ParticleFile)
	: AssetEditor("Particle: %s", ParticleFile)
{
	PreviewScene = new Scene();
	Toolbar* PanelToolbar = new Toolbar();

	Background->AddChild(PanelToolbar);

	SideBarBox = new UIScrollBox(false, 0, true);
	MainBox = new UIBox(true);
	Background->AddChild(MainBox);

	PreviewElement = new ScenePreviewElement(PreviewScene, SizeVec::Pixels(300, 200));
	PreviewElement->SetBorder(1_px, EditorUI::Theme.Background);

	PreviewObject = PreviewScene->CreateObject<ParticleObject>();
	PreviewObject->ParticleFile.Value = ParticleFile;
	PreviewObject->ParticleFile.OnChanged();
	PreviewObject->Particles->DoHotReload = false;

	PreviewElement->SetPadding(5_px);
	SideBarBox->AddChild(PreviewElement);

	SideBarBackground = new UIBackground(false, 0, EditorUI::Theme.DarkBackground);
	SideBarBackground->SetLeftPadding(1_px);
	SideBarBackground->SetCorner(EditorUI::Theme.CornerSize);
	SideBarBackground->SetCorners(false, false, true, false);

	ContentBox = new UIScrollBox(false, 0, true);
	MainBox->AddChild(SideBarBackground->AddChild(SideBarBox));
	MainBox->AddChild(ContentBox);
	ContentBox->SetPadding(5_px);


	BoundsShape = new debug::DebugBox(Transform(), Vector3(0.5f, 0.1f, 0.2f));

	PanelToolbar->AddButton("Save", EditorUI::Asset("Save.png"), [this] {
		Save();
	});

	PanelToolbar->AddDropdown("View", EditorUI::Asset("Options.png"), [this] {
		return std::vector{
			DropdownMenu::Option{
				.Name = "Show Bounding Box",
				.Icon = ShowBounds ? EditorUI::Asset("Check.png") : "",
				.OnClicked = [this] {
					ShowBounds = !ShowBounds;
				}
			}
		};
	});

	SideBarMenu = new PropertyMenu();

	SideBarBox->AddChild(SideBarMenu);
}

engine::editor::ParticleEditor::~ParticleEditor()
{
	PreviewScene->Graphics.Debug.RemoveShape(BoundsShape);
	delete BoundsShape;
	delete PreviewScene;
}

void engine::editor::ParticleEditor::OnResized()
{
	auto TotalSize = UISize(Size.X).GetPixels().X;

	bool Compact = TotalSize < 600;

	MainBox->SetHorizontal(!Compact);
	SideBarBox->SetHorizontal(Compact);

	if (Compact)
	{
		SideBarBox->SetSize(SizeVec(Size.X - (5_px).GetScreen().X, 200_px));
		SideBarMenu->SetMinWidth(Size.X - (310_px).GetScreen().X);
		ContentBox->SetSize(Size - SizeVec::Pixels(10, 250).GetScreen());
	}
	else
	{
		SideBarBox->SetSize(SizeVec(310_px, Size.Y - (42_px).GetScreen().Y));
		ContentBox->SetSize(Size - SizeVec::Pixels(310, 60).GetScreen());
		SideBarMenu->SetMinWidth(UISize::Parent(1));
	}
	Background->UpdateElement();
	UpdateSideBar();
	BuildLayout();
}

void engine::editor::ParticleEditor::OnThemeChanged()
{
	UpdateSideBar();
	BuildLayout();
	PreviewElement->SetBorder(1_px, EditorUI::Theme.Background);
	SideBarBackground->SetColor(EditorUI::Theme.DarkBackground);
	SideBarBackground->SetCorner(EditorUI::Theme.CornerSize);
}

void engine::editor::ParticleEditor::Update()
{
	PreviewElement->AlwaysRedraw = Visible;

	if (ShowBounds)
	{
		const BoundingBox& Bounds = PreviewObject->Particles->DrawBoundingBox;
		BoundsShape->CubeTransform = Transform(Bounds.Position, 0, Bounds.Extents);

		if (!IsBoundsShapeAdded)
		{
			IsBoundsShapeAdded = true;
			PreviewScene->Graphics.Debug.AddShape(BoundsShape);
		}
	}
	else if (IsBoundsShapeAdded)
	{
		PreviewScene->Graphics.Debug.RemoveShape(BoundsShape);
		IsBoundsShapeAdded = false;
	}

	if (AutoRepeat && !PreviewObject->Particles->IsPlaying())
	{
		PreviewObject->Particles->Reset();
	}
}

void engine::editor::ParticleEditor::Save()
{
	AssetEditor::Save();

	std::stringstream FileString;
	auto Data = PreviewObject->Particles->Data->Serialize();
	TextSerializer::ToStream(Data.GetObject(), FileString);

	BufferStream Buffer;
	Buffer.WriteStringNoNull(FileString.str());
	Buffer.ResetStreamPosition();
	EditorUI::Instance->AssetsProvider->SaveToFile(EditedAsset.FilePath, &Buffer, Buffer.GetSize());
	EditorUI::Instance->OnProjectAssetChanged(EditedAsset);
}

void engine::editor::ParticleEditor::BuildLayout()
{
	ContentBox->DeleteChildren();

	int Counter = 0;

	UIBox* Row = nullptr;
	bool Attached = false;

	int64 Count = std::max(int64(UISize(Size.X).GetPixels().X / 310.0f ) - 1, int64(1));

	auto AddElement = [&Row, &Counter, Count, &Attached, this](UIBox* NewElement) {
		if (Counter % Count == 0)
		{
			if (Row)
			{
				ContentBox->AddChild(Row);
			}
			Row = new UIBox(true, 0);
		}

		Row->AddChild(NewElement);
		Counter++;
	};

	auto OnValueChanged = [this] {
		this->OnChanged();
		PreviewObject->Particles->Reset();
	};

	for (auto& i : PreviewObject->Particles->Data->Emitters)
	{
		auto Item = new ParticleEditorItem();

		Item->SetName(str::Format("Element %i", Counter + 1));

		auto Properties = new PropertyMenu();
		Properties->SetSize(UISize::Parent(1));
		Properties->SetPadding(5_px);
		Item->content->AddChild(Properties);
		Item->close->OnClicked = [this, Counter, OnValueChanged] {
			std::lock_guard g{ PreviewObject->Particles->Data->EmittersMutex };
			PreviewObject->Particles->RemoveEmitter(Counter);
			OnValueChanged();
			BuildLayout();
		};

		AddElement(Item);
		Properties->CreateNewHeading("Appearance");
		Properties->AddAssetRefEntry("Material", i.Data.MaterialFile, [&i, OnValueChanged] {
			i.Data.ReloadMaterial();
			OnValueChanged();
		});

		Properties->CreateNewHeading("Transform");
		Properties->AddVecEntry("Start Position", i.Data.StartPosition, OnValueChanged);
		Properties->AddVecEntry("Position Random", i.Data.StartRandom, OnValueChanged);
		Properties->AddFloatEntry("Rotation", i.Data.Rotation, OnValueChanged);
		Properties->AddFloatEntry("Start Scale", i.Data.StartScale, OnValueChanged);
		Properties->AddFloatEntry("End Scale", i.Data.EndScale, OnValueChanged);

		Properties->CreateNewHeading("Velocity");
		Properties->AddVecEntry("Start Velocity", i.Data.Velocity, OnValueChanged);
		Properties->AddVecEntry("Velocity Random", i.Data.VelocityRandom, OnValueChanged);
		Properties->AddVecEntry("Acceleration", i.Data.Acceleration, OnValueChanged);
		Properties->AddFloatEntry("Rotation Start Velocity", i.Data.RotationVelocity, OnValueChanged);
		Properties->AddFloatEntry("Rotation Velocity Random", i.Data.RotationVelocityRandom, OnValueChanged);
		Properties->CreateNewHeading("Spawn");
		Properties->AddFloatEntry("Lifetime", i.Data.Lifetime, OnValueChanged);
		Properties->AddFloatEntry("Spawn Rate", i.Data.SpawnDelay, OnValueChanged);
		Properties->AddIntEntry("Spawn Count", i.Data.SpawnCount, OnValueChanged);
	}

	auto NewElementButton = new ParticleEditorNewItemCard();
	NewElementButton->btn->OnClicked = [this, OnValueChanged] {
		std::lock_guard g{ PreviewObject->Particles->Data->EmittersMutex };
		PreviewObject->Particles->InitializeEmitter(&PreviewObject->Particles->Data->Emitters.emplace_back());
		OnValueChanged();
		BuildLayout();
	};
	AddElement(NewElementButton);

	ContentBox->AddChild(Row);
}

void engine::editor::ParticleEditor::UpdateSideBar()
{
	SideBarMenu->DeleteChildren();
	SideBarMenu->CreateNewHeading("Preview");

	SideBarMenu->AddBooleanEntry("Repeat When Finished", AutoRepeat, nullptr);
	SideBarMenu->AddButtonEntry("Reset Playback", "Reset", [this] {
		PreviewObject->Particles->Reset();
	});

	SideBarMenu->CreateNewHeading("Performance");
	SideBarMenu->AddBooleanEntry("Automatic Bounding Box",
		PreviewObject->Particles->Data->AutomaticBoundingVolume, [] {

	});
}
