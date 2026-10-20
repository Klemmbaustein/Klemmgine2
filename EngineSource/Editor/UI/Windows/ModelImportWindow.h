#pragma once
#include "IDialogWindow.h"
#include <Editor/UI/Windows/ProgressBar.h>

namespace engine::editor
{
	class ModelImporterWindow : public IDialogWindow
	{
	public:
		ModelImporterWindow(std::vector<string> ModelFiles, const string& ToPath);

		// Inherited via IDialogWindow
		void Begin() override;
		void Update() override;
		void Destroy() override;

		static inline std::vector<string> ModelExtensions = { "glb", "gltf", "obj", "fbx", "ply",
			"blend", "raw", "stl", "3ds", "amf", "assbin", "b3d", "dfx" };

	private:
		float ImportScale = 1.0f;
		bool ImportMaterials = true;
		bool ImportTextures = true;
		bool GenerateUV = true;
		bool OptimizeMeshData = true;

		void DoImport();
		void ImportFile(const string& FilePath, ProgressBar* Progress, float ProgressOffset);

		std::vector<string> ModelFiles;
		string ToPath;
	};
}