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
using namespace engine;
using namespace engine::graphics;
using engine::subsystem::VideoSubsystem;

const auto number = std::stoi("15");

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
	PreviewScene->AlwaysRedraw = false;
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
	Sidebar = new UIBackground(false, 0, EditorUI::Theme.DarkBackground);

	MainBox = new UIBox(true);

	Background->AddChild(MaterialToolbar);
	Background->AddChild(MainBox
		->AddChild(Sidebar
			->SetCorner(5_px)
			->SetPadding(0, 1_px, 1_px, 1_px)
			->AddChild(PreviewImage
				->SetPadding(5_px)))
		->AddChild(MaterialParamsBox));

	UIBox* PropertiesBox = new UIBox(false);

	auto AddSidebarTextField = [PropertiesBox, this](string Name, string& Value)
	{
		auto ShaderField = new UITextField(0, EditorUI::Theme.Background, EditorUI::EditorFont, nullptr);
		ShaderField
			->SetText(Value)
			->SetTextColor(EditorUI::Theme.Text)
			->SetMinSize(SizeVec(130_px, UISize::Parent(1)));
		ShaderField->OnChanged = [this, ShaderField, &Value]()
		{
			if (Value == ShaderField->GetText())
				return;
			Value = ShaderField->GetText();
			this->LoadedMaterial->UpdateShader();
			this->OnChanged();
			this->LoadUI();
		};

		PropertiesBox->AddChild((new UIBox(true))
			->SetPadding(5_px)
			->SetVerticalAlign(UIBox::Align::Centered)
			->AddChild((new UIText(11_px, EditorUI::Theme.Text, Name, EditorUI::EditorFont))
				->SetTextWidthOverride(65_px)
				->SetWrapEnabled(true, 65_px))
			->AddChild(ShaderField));
	};

	Sidebar->AddChild(PropertiesBox);

	AddSidebarTextField("Vertex shader", LoadedMaterial->VertexShader);
	AddSidebarTextField("Fragment shader", LoadedMaterial->FragmentShader);
}

engine::editor::MaterialEditor::~MaterialEditor()
{
	if (PreviewScene)
		delete PreviewScene;
}

void engine::editor::MaterialEditor::Update()
{
	AssetEditor::Update();
	PreviewImage->SetUseImage(true, PreviewScene->GetDrawBuffer());

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
	Sidebar->SetHorizontal(Compact);

	if (Compact)
	{
		Sidebar->SetCorners(false, false, false, false);
		Sidebar->SetMinSize(SizeVec(UISize::Parent(1), 0));
	}
	else
	{
		Sidebar->SetCorners(false, false, true, false);
		Sidebar->SetMinSize(SizeVec(0, UISize::Parent(1)));
	}

	for (Material::Field& i : LoadedMaterial->Fields)
	{
		auto Editor = new MaterialPropertyElement();
		Editor->SetName(i.Name);
		MaterialParamsBox->AddChild(Editor);

		string TypeName;
		switch (i.FieldType)
		{
		case Material::Field::Type::Bool:
			TypeName = "bool";
			break;
		case Material::Field::Type::Float:
			TypeName = "bool";
			break;
		case Material::Field::Type::Int:
			TypeName = "bool";
			break;
		case Material::Field::Type::Texture:
			TypeName = "sampler2D";
			break;
		case Material::Field::Type::Vec3:
			TypeName = "vec3";
			break;
		default:
			break;
		}
		Editor->SetDescription(TypeName);

		if (i.FieldType == Material::Field::Type::Texture)
		{
			CreateTextureField(Editor->contentBox, i);
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
	}
}
void engine::editor::MaterialEditor::Save()
{
	AssetEditor::Save();
	LoadedMaterial->ToFile(EditedAsset.FilePath);
}
void engine::editor::MaterialEditor::OnResized()
{
	MainBox->SetMinSize(SizeVec(Size.X, Size.Y - (41_px).GetScreen().Y));
	LoadUI();
}

void engine::editor::MaterialEditor::OnChanged()
{
	AssetEditor::OnChanged();
	PreviewScene->Redraw = true;
	RedrawNextFrame = true;
}

void engine::editor::MaterialEditor::CreateTextureField(UIBox* Parent, Material::Field& Field)
{
	Material::MatTexture TextureName;

	if (Field.TextureValue.Name)
	{
		TextureName = *Field.TextureValue.Name;
	}

	auto TexElement = new MaterialTextureElement();

	auto Selector = new AssetSelector(
		TextureName.Name.empty() ? AssetRef{ .Extension = "png" } : AssetRef::Convert(TextureName.Name),
		UISize(180_px).GetScreen().X, nullptr);

	Selector->OnChanged = [this, Selector, TexElement, &Field]()
	{
		string Name = file::FileName(Selector->SelectedAsset.FilePath);

		if (!Field.TextureValue.Name)
		{
			Field.TextureValue.Name = new Material::MatTexture(Name);
		}
		else
		{
			Field.TextureValue.Name->Name = Name;
			Field.TextureValue.Value = nullptr;
		}
		OnChanged();
	};

	LoadedMaterial->LoadTexture(Field);
	TexElement->texturePreview->SetUseTexture(Field.TextureValue.Value, Field.TextureValue.Value
		? Field.TextureValue.Value->TextureObject : 0);
	Selector->SelectedAsset.Extension = "png";

	TexElement->controlsBox->AddChild(Selector);

	if (Field.TextureValue.Name)
	{
		UIBox* ControlsBox = new UIBox(true);

		auto CreateTextureDropdown = [this, TexElement, ControlsBox, &Field]
		(std::vector<UIDropdown::Option> FilteringOptions, uint8 DefaultValue, std::function<void(uint8)> SetValue)
		{
			auto Dropdown = new UIDropdown(0, 75_px, EditorUI::Theme.DarkBackground,
				EditorUI::Theme.Text, FilteringOptions, nullptr, EditorUI::EditorFont);
			Dropdown->SelectOption(DefaultValue, false);
			Dropdown->OnClicked = [this, Dropdown, TexElement, &Field, SetValue]()
			{
				SetValue(uint8(Dropdown->SelectedIndex));
				Field.TextureValue.Value = nullptr;
				TexElement->texturePreview->RedrawElement();

				OnChanged();
			};

			Dropdown->SetPadding(5_px);
			Dropdown->SetTextSize(11_px, 3_px);
			Dropdown->SetCorner(5_px);
			ControlsBox->AddChild(Dropdown);
		};

		TexElement->controlsBox->AddChild(ControlsBox);

		CreateTextureDropdown({
			UIDropdown::Option("Nearest"),
			UIDropdown::Option("Linear"),
			}, TextureName.Options.Filter, [&Field](uint8 NewValue)
		{
			Field.TextureValue.Name->Options.Filter = TextureOptions::Filtering(NewValue);
		});

		CreateTextureDropdown({
				UIDropdown::Option("Border"),
				UIDropdown::Option("Clamp"),
				UIDropdown::Option("Repeat")
			}, TextureName.Options.TextureBorders, [&Field](uint8 NewValue)
		{
			Field.TextureValue.Name->Options.TextureBorders = TextureOptions::BorderType(NewValue);
		});
	}

	Parent->AddChild(TexElement);
}
#endif