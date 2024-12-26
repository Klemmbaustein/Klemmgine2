#ifdef EDITOR
#pragma once
#include <Engine/Editor/UI/Panels/EditorPanel.h>
#include <Engine/File/AssetRef.h>
#include <Engine/Graphics/Material.h>

namespace engine::editor
{
	class MaterialEditor : public EditorPanel
	{
	public:
		MaterialEditor(AssetRef MaterialFile);

		graphics::Material LoadedMaterial;
	};
}
#endif