#pragma once
#include <Core/Types.h>
#include <functional>

namespace engine::editor::modelConverter
{
	struct ConvertOptions
	{
		/// Optimize the mesh by removing duplicate vertices, empty models, etc.
		bool Optimize : 1 = true;
		/// Automatically generate UV coordinates if they are missing.
		bool GenerateUV : 1 = true;
		/// Import texture files.
		bool ImportTextures : 1 = true;
		/// Import and create material files.
		bool ImportMaterials : 1 = true;

		/// Import scale to multiply everything by.
		float ImportScale = 1;

		/// Function called when the import status changes, used for displaying a loading bar with a status message.
		std::function<void(string)> OnLoadStatusChanged;
	};

	/**
	 * @brief
	 * Converts a regular 3D model file into the engine's 3D model, texture and material format.
	 * @param ModelPath
	 * The path to the model file.
	 * @param OutDir
	 * The directory to put the resulting model and material files in.
	 * @param Options
	 * Conversion options.
	 * @return
	 * The path of the resulting model file.
	 */
	string ConvertModel(string ModelPath, string OutDir, ConvertOptions Options);
}