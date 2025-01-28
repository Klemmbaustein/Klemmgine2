#ifdef EDITOR
#include "MaterialEditor.h"
#include <Engine/Editor/UI/Elements/AssetSelector.h>
#include <MaterialEditor.kui.hpp>
#include <Engine/Objects/MeshObject.h>
#include <Engine/File/FileUtil.h>

using namespace kui;

engine::editor::MaterialEditor::MaterialEditor(AssetRef MaterialFile)
	: EditorPanel("Material: " + MaterialFile.DisplayName(), "")
{
	LoadedMaterial = new graphics::Material(MaterialFile);

	Background->SetHorizontal(true);

	PreviewScene = new Scene();
	PreviewScene->Physics.Active = false;
	PreviewScene->SceneCamera->Rotation = Vector3(-0.7f, -2.4f, 0);
	PreviewScene->SceneCamera->Position = Vector3(3, 3, 3);
	PreviewScene->Resizable = false;
	PreviewScene->AlwaysRedraw = true;
	PreviewScene->Redraw = true;
	PreviewScene->OnResized(400);

	auto CurrentObj = PreviewScene->CreateObject<MeshObject>();
	CurrentObj->LoadMesh("cube.kmdl"_asset);
	CurrentObj->Mesh->Materials[0] = LoadedMaterial;

	MaterialParamsBox = new UIScrollBox(false, 0, true);
	MaterialParamsBox->SetMinHeight(UISize(1, SizeMode::ParentRelative));
	MaterialParamsBox->SetMaxHeight(UISize(1, SizeMode::ParentRelative));
	MaterialParamsBox->SetPadding(5_px);
	Background->AddChild(MaterialParamsBox);

	PreviewImage = new UIBackground(true, 0, 1, 400_px);
	Background->AddChild(PreviewImage);

	LoadUI();
}
engine::editor::MaterialEditor::~MaterialEditor()
{
	if (PreviewScene)
		delete PreviewScene;
}

void engine::editor::MaterialEditor::Update()
{
	PreviewImage->SetUseTexture(true, PreviewScene->Buffer->Textures[0]);
}

void engine::editor::MaterialEditor::LoadUI()
{
	MaterialParamsBox->DeleteChildren();
	for (auto& i : LoadedMaterial->Fields)
	{
		auto Editor = new MaterialPropertyElement();
		Editor->SetName(i.Name);

		if (i.FieldType == graphics::Material::Field::Type::Texture)
		{
			std::string TextureName;

			if (i.TextureValue.Name)
			{
				TextureName = *i.TextureValue.Name;
			}

			auto Field = new MaterialTextureElement();

			auto Selector = new AssetSelector(AssetRef::Convert(TextureName), UISize(200_px).GetScreen().X, nullptr);

			Selector->OnChanged = [this, Selector, &i]()
				{
					if (i.TextureValue.Name)
						delete i.TextureValue.Name;

					i.TextureValue.Name = new string(file::FileName(Selector->SelectedAsset.FilePath));
					i.TextureValue.Value = nullptr;
					PreviewScene->Redraw = true;
					PreviewImage->RedrawElement();
				};

			Selector->SelectedAsset.Extension = "png";

			Field->controlsBox->AddChild(Selector);
			Editor->contentBox->AddChild(Field);
			MaterialParamsBox->AddChild(Editor);
			continue;
		}
		auto Field = new MaterialValueElement();
		Editor->contentBox->AddChild(Field);
		MaterialParamsBox->AddChild(Editor);
	}
}
#endif