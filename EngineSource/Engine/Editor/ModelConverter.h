#pragma once
#include <Engine/Types.h>

namespace engine::editor::modelConverter
{
	struct ConvertOptions
	{
		bool Optimize : 1 = true;
		bool GenerateUV : 1 = true;
		bool ImportTextures : 1 = true;
	};

	string ConvertModel(string ModelPath, string OutDir, ConvertOptions Options);
}
