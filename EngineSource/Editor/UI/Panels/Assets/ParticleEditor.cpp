#include "ParticleEditor.h"
#include <ParticleEditor.kui.hpp>
#include <Editor/UI/Elements/PropertyMenu.h>
#include <kui/UI/UIScrollBox.h>

engine::editor::ParticleEditor::ParticleEditor(AssetRef ParticleFile)
	: AssetEditor("Particle: %s", ParticleFile)
{
	PreviewScene = new Scene();

	PreviewObject = PreviewScene->CreateObject<ParticleObject>();
	PreviewObject->ParticleFile.Value = ParticleFile;
	PreviewObject->ParticleFile.OnChanged();

	PreviewElement = new ScenePreviewElement(PreviewScene, kui::SizeVec::Pixels(300, 200));
	PreviewElement->AlwaysRedraw = true;
	PreviewElement->SetPadding(5_px);
	Background->AddChild(PreviewElement);

	ContentBox = new kui::UIScrollBox(false, 0, true);
	Background->AddChild(ContentBox);
	ContentBox->SetPadding(5_px);
}

engine::editor::ParticleEditor::~ParticleEditor()
{
	delete PreviewScene;
}

void engine::editor::ParticleEditor::OnResized()
{
	BuildLayout();
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
		Properties->SetMinWidth(240_px);
		Properties->SetPadding(5_px);
		Item->content->AddChild(Properties);

		Row->AddChild(Item);
		Properties->AddVecEntry("Start Velocity", i.Data.Velocity, nullptr);
		Properties->AddFloatEntry("Start Scale", i.Data.StartScale, nullptr);
		Properties->AddFloatEntry("End Scale", i.Data.EndScale, nullptr);
	}

	ContentBox->AddChild(Row);
}
