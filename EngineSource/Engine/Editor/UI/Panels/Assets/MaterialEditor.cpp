#include "MaterialEditor.h"

engine::editor::MaterialEditor::MaterialEditor(AssetRef MaterialFile)
	: EditorPanel("Material: " + MaterialFile.DisplayName(), "")
{
}
