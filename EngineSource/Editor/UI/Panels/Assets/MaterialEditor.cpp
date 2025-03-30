#ifdef EDITOR
#include "MaterialEditor.h"
#include <Editor/UI/Elements/AssetSelector.h>
#include <Editor/UI/Elements/Checkbox.h>
#include <Editor/UI/Elements/VectorField.h>
#include <Editor/UI/Elements/Toolbar.h>
#include <MaterialEditor.kui.hpp>
#include <Engine/Objects/MeshObject.h>
#include <Core/File/FileUtil.h>
#include <stdexcept>
#include <kui/Window.h>
#include <Engine/Subsystem/VideoSubsystem.h>
#include <Engine/Engine.h>
#include <kui/UI/UIDropdown.h>

using namespace kui;
using engine::subsystem::VideoSubsystem;

engine::editor::MaterialEditor::MaterialEditor(AssetRef MaterialFile)
	: AssetEditor("Material: %s", MaterialFile)
{
	LoadedMaterial = new graphics::Material(MaterialFile);

	Background->SetHorizontal(false);

	PreviewScene = new Scene();
	PreviewScene->Physics.Active = false;
	PreviewScene->SceneCamera->Rotation = Vector3(-0.7f, -2.4f, 0);
	PreviewScene->SceneCamera->Position = Vector3(3, 3, 3);
	PreviewScene->Resizable = false;
	PreviewScene->AlwaysRedraw = true;
	PreviewScene->Redraw = true;
	PreviewScene->BufferSize = 200 * Window::GetActiveWindow()->GetDPI();
	PreviewScene->OnResized(PreviewScene->BufferSize);

	auto CurrentObj = PreviewScene->CreateObject<MeshObject>();
	CurrentObj->LoadMesh("cube.kmdl"_asset);
	CurrentObj->Mesh->Materials[0] = LoadedMaterial;

	MaterialParamsBox = new UIScrollBox(false, 0, true);
	MaterialParamsBox->SetMinHeight(UISize(1, SizeMode::ParentRelative));
	MaterialParamsBox->SetMaxHeight(UISize(1, SizeMode::ParentRelative));
	MaterialParamsBox->SetPadding(5_px);

	Toolbar* MaterialToolbar = new Toolbar();
	MaterialToolbar->AddButton("Save", "file:Engine/Editor/Assets/Save.png",
		[this]()
		{
			Save();
		});
	MaterialToolbar->AddButton("Reload shaders", "file:Engine/Editor/Assets/Reload.png",
		[this]()
		{
			Engine::GetSubsystem<VideoSubsystem>()->Shaders.ReloadAll();
		});

	PreviewImage = new UIBackground(true, 0, 1, 200_px);
	Sidebar = new UIBackground(false, 0, EditorUI::Theme.Background);

	MainBox = new UIBox(true);

	Background->AddChild(MaterialToolbar);
	Background->AddChild(MainBox
		->SetMinHeight(UISize::Parent(1).GetScreen().Y - (42_px).GetScreen().Y)
		->AddChild(Sidebar
			->SetPadding(1_px)
			->SetMinHeight(UISize::Parent(1))
			->AddChild(PreviewImage
				->SetPadding(5_px)))
		->AddChild(MaterialParamsBox));

	LoadUI();
}
engine::editor::MaterialEditor::~MaterialEditor()
{
	if (PreviewScene)
		delete PreviewScene;
}

void engine::editor::MaterialEditor::Update()
{
	AssetEditor::Update();
	PreviewImage->SetUseTexture(true, PreviewScene->Buffer->Textures[0]);

	if (RedrawNextFrame)
	{
		PreviewImage->RedrawElement();
		RedrawNextFrame = false;
	}
}

void engine::editor::MaterialEditor::LoadUI()
{
	using graphics::Material;

	MaterialParamsBox->DeleteChildren();

	bool Compact = SizeVec(Size).GetPixels().X < 600;

	MainBox->SetHorizontal(!Compact);

	if (Compact)
	{
		Sidebar->SetMinSize(SizeVec(UISize::Parent(1), 0));
	}
	else
	{
		Sidebar->SetMinSize(SizeVec(0, UISize::Parent(1)));
	}

	for (Material::Field& i : LoadedMaterial->Fields)
	{
		auto Editor = new MaterialPropertyElement();
		Editor->SetName(i.Name);

		if (i.FieldType == Material::Field::Type::Texture)
		{
			string TextureName;

			if (i.TextureValue.Name)
			{
				TextureName = *i.TextureValue.Name;
			}

			auto Field = new MaterialTextureElement();

			auto Selector = new AssetSelector(TextureName.empty() ? AssetRef{.Extension = "png"} : AssetRef::Convert(TextureName), UISize(200_px).GetScreen().X, nullptr);

			Selector->OnChanged = [this, Selector, &i]()
				{
					if (i.TextureValue.Name)
						delete i.TextureValue.Name;

					i.TextureValue.Name = new string(file::FileName(Selector->SelectedAsset.FilePath));
					i.TextureValue.Value = nullptr;
					OnChanged();
				};

			LoadedMaterial->LoadTexture(i);
			Field->texturePreview->SetUseTexture(i.TextureValue.Value, i.TextureValue.Value
				? i.TextureValue.Value->TextureObject : 0);
			Selector->SelectedAsset.Extension = "png";
			 
			std::vector FilteringOptions = {
				UIDropdown::Option("Nearest"),
				UIDropdown::Option("Linear"),
			};

			auto MaterialFilteringDropdown = new UIDropdown(0, 50_px, 0, 1, FilteringOptions, [](int Selected)
				{

				}, EditorUI::EditorFont);

			MaterialFilteringDropdown->SetTextSize(11_px, 1_px);

			Field->controlsBox->AddChild(Selector);
			Field->controlsBox->AddChild(MaterialFilteringDropdown);
			Editor->contentBox->AddChild(Field);
			MaterialParamsBox->AddChild(Editor);
			continue;
		}

		if (i.FieldType == Material::Field::Type::Vec3)
		{
			auto VecField = new VectorField(i.Vec3, 200_px, nullptr);
			VecField->OnChanged = [this, VecField, &i]()
				{
					if (i.Vec3 == VecField->GetValue())
						return;
					i.Vec3 = VecField->GetValue();
					OnChanged();
				};


			Editor->contentBox->AddChild(VecField);
			MaterialParamsBox->AddChild(Editor);
			continue;
		}
		if (i.FieldType == Material::Field::Type::Bool)
		{
			auto Checkbox = new UICheckbox(i.Int, nullptr);
			Checkbox->OnClicked = [this, Checkbox, &i]()
				{
					i.Int = Checkbox->Value;
					OnChanged();
				};

			Editor->contentBox->AddChild(Checkbox);
			MaterialParamsBox->AddChild(Editor);
			continue;
		}

		auto Field = new MaterialValueElement();
		switch (i.FieldType)
		{
		case Material::Field::Type::Int:
		case Material::Field::Type::Bool:
			Field->field->field->SetText(std::to_string(i.Int));
			Field->field->field->OnChanged = [this, field = Field->field->field, &i]()
				{
					try
					{
						i.Int = std::stoi(field->GetText());
					}
					catch (std::out_of_range e) {}
					OnChanged();
				};
			break;
		case Material::Field::Type::Float:
			Field->field->field->SetText(std::to_string(i.Float));
			Field->field->field->OnChanged = [this, field = Field->field->field, &i]()
				{
					try
					{
						i.Float = std::stof(field->GetText());
					}
					catch (std::out_of_range e) {}
					OnChanged();
				};
			break;
		default:
			break;
		}
		Editor->contentBox->AddChild(Field);
		MaterialParamsBox->AddChild(Editor);
	}
}
void engine::editor::MaterialEditor::Save()
{
	AssetEditor::Save();
	LoadedMaterial->ToFile(EditedAsset.FilePath);
}
void engine::editor::MaterialEditor::OnResized()
{
	LoadUI();
}
void engine::editor::MaterialEditor::OnChanged()
{
	AssetEditor::OnChanged();
	PreviewScene->Redraw = true;
	RedrawNextFrame = true;
}
#endif