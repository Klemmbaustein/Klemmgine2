#include "ParticleEditor.h"
#include <ParticleEditor.kui.hpp>
#include <Editor/UI/Elements/PropertyMenu.h>
#include <kui/UI/UIScrollBox.h>
#include <Editor/UI/Elements/Toolbar.h>

engine::editor::ParticleEditor::ParticleEditor(AssetRef ParticleFile)
	: AssetEditor("Particle: %s", ParticleFile)
{
	PreviewScene = new Scene();

	PreviewObject = PreviewScene->CreateObject<ParticleObject>();
	PreviewObject->ParticleFile.Value = ParticleFile;
	PreviewObject->ParticleFile.OnChanged();
	Toolbar* PanelToolbar = new Toolbar();

	Background->AddChild(PanelToolbar);

	PreviewElement = new ScenePreviewElement(PreviewScene, kui::SizeVec::Pixels(300, 200));
	PreviewElement->AlwaysRedraw = true;
	PreviewElement->SetPadding(5_px);
	Background->AddChild(PreviewElement);

	ContentBox = new kui::UIScrollBox(false, 0, true);
	Background->AddChild(ContentBox);
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
}

engine::editor::ParticleEditor::~ParticleEditor()
{
	PreviewScene->Graphics.Debug.RemoveShape(BoundsShape);
	delete BoundsShape;
	delete PreviewScene;
}

void engine::editor::ParticleEditor::OnResized()
{
	BuildLayout();
}

void engine::editor::ParticleEditor::Update()
{
	if (ShowBounds)
	{
		const auto& Bounds = PreviewObject->Particles->Data->Emitters[0].Bounds;
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
}

void engine::editor::ParticleEditor::BuildLayout()
{
	ContentBox->DeleteChildren();

	int Counter = 1;

	auto Row = new kui::UIBox(true, 0);

	for (auto& i : PreviewObject->Particles->Data->Emitters)
	{
		auto Item = new ParticleEditorItem();

		Item->SetName(str::Format("Element %i", Counter++));

		auto Properties = new PropertyMenu();
		Properties->SetSize(kui::SizeVec(240_px, kui::UISize::Parent(1)));
		Properties->SetPadding(5_px);
		Item->content->AddChild(Properties);

		Row->AddChild(Item);
		Properties->CreateNewHeading("Position");
		Properties->AddVecEntry("Start Position", i.Data.StartPosition, nullptr);
		Properties->AddVecEntry("Position Random", i.Data.StartRandom, nullptr);

		Properties->CreateNewHeading("Velocity");
		Properties->AddVecEntry("Start Velocity", i.Data.Velocity, nullptr);
		Properties->AddVecEntry("Velocity Random", i.Data.VelocityRandom, nullptr);
		Properties->AddVecEntry("Acceleration", i.Data.Acceleration, nullptr);
		Properties->CreateNewHeading("Scale");
		Properties->AddFloatEntry("Start Scale", i.Data.StartScale, nullptr);
		Properties->AddFloatEntry("End Scale", i.Data.EndScale, nullptr);
		Properties->CreateNewHeading("Spawn");
		Properties->AddFloatEntry("Lifetime", i.Data.Lifetime, nullptr);
	}

	ContentBox->AddChild(Row);
}
