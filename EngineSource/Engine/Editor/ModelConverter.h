#ifdef EDITOR
#pragma once
#include <Engine/Types.h>
#include <functional>

namespace engine::editor::modelConverter
{
	struct ConvertOptions
	{
		bool Optimize : 1 = true;
		bool GenerateUV : 1 = true;
		bool ImportTextures : 1 = true;
		bool ImportMaterials : 1 = true;

		float ImportScale = 0.01f;

		std::function<void(string)> OnLoadStatusChanged;
	};

	string ConvertModel(string ModelPath, string OutDir, ConvertOptions Options);
}
#endif