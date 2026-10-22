#include "ModelImportWindow.h"
#include <Editor/UI/Elements/PropertyMenu.h>
#include <Editor/ModelConverter.h>
#include <Core/File/FileUtil.h>
#include <Engine/MainThread.h>
#include <Engine/File/Resource.h>
#include <Editor/UI/Panels/AssetBrowser.h>

using namespace kui;

engine::editor::ModelImporterWindow::ModelImporterWindow(std::vector<string> ModelFiles, const string& ToPath)
	: IDialogWindow("Import model file",
		DefaultOkCancelOptions(std::bind(&ModelImporterWindow::DoImport, this)), Vec2ui(340, 400)),
	ModelFiles(std::move(ModelFiles)),
	ToPath(std::move(ToPath))
{
	Open();
}

void engine::editor::ModelImporterWindow::Begin()
{
	IDialogWindow::Begin();

	auto Properties = new PropertyMenu();
	Properties->SetSize(UISize::Parent(1));
	Properties->SetPadding(0, 0, 10_px, 10_px);

	Background->AddChild(Properties);
	if (ModelFiles.size() > 1)
	{
		Properties->CreateNewHeading(str::Format("Import %i 3D Model Files", ModelFiles.size()));
	}
	else
	{
		Properties->CreateNewHeading("Import 3D Model File");
	}

	Properties->AddFloatEntry("Scale", ImportScale, nullptr);
	Properties->AddBooleanEntry("Import materials", ImportMaterials, nullptr);
	Properties->AddBooleanEntry("Import textures", ImportTextures, nullptr);
	Properties->AddBooleanEntry("Optimize mesh", OptimizeMeshData, nullptr);
	Properties->AddBooleanEntry("Generate UV if missing", GenerateUV, nullptr);
}

void engine::editor::ModelImporterWindow::Update()
{
}

void engine::editor::ModelImporterWindow::Destroy()
{
}

void engine::editor::ModelImporterWindow::DoImport()
{
	float ProgressValue = ModelFiles.size() == 1 ? -1 : 0;

	auto* Progress = new ProgressBar("Importing model file");
	for (const auto& i : this->ModelFiles)
	{
		ImportFile(i, Progress, ProgressValue);
		ProgressValue += 1.0f / float(ModelFiles.size());
	}
	Progress->Close();
}

void engine::editor::ModelImporterWindow::ImportFile(const string& FilePath, ProgressBar* Progress, float ProgressOffset)
{
	string Extension = file::Extension(FilePath);

	EditorUI::SetStatusMessage(str::Format("Importing file: '%s'", FilePath.c_str()), EditorUI::StatusType::Info);
	Progress->Progress = ProgressOffset;
	string Out = modelConverter::ConvertModel(FilePath, ToPath, modelConverter::ConvertOptions{
		.Optimize = OptimizeMeshData,
		.GenerateUV = GenerateUV,
		.ImportTextures = ImportTextures,
		.ImportMaterials = ImportMaterials,
		.ImportScale = ImportScale,
		.OnLoadStatusChanged = [Progress](string NewMessage) {
			Progress->SetMessage("Importing model: " + NewMessage);
		},
		});
	EditorUI::SetStatusMessage(str::Format("Imported '%s' to '%s'", FilePath.c_str(), Out.c_str()), EditorUI::StatusType::Info);

	thread::ExecuteOnMainThread([Out] {
		resource::ScanForAssets();
		EditorUI::Instance->OnProjectAssetChanged(AssetRef::FromPath(Out));
		EditorUI::ForEachPanel<AssetBrowser>([](AssetBrowser* Browser) {
			Browser->UpdateItems();
		});
	});
}
