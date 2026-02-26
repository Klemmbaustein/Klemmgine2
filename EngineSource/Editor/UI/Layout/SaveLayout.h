#pragma once
#include <Core/Types.h>
#include <Editor/UI/Panels/EditorPanel.h>
#include <Editor/UI/Panels/PanelRegistry.h>

namespace engine::editor::layout
{
	void LayoutToFile(EditorPanel* Root, string File);
	void LoadLayout(EditorPanel* Root, string File, PanelRegistry* Registry);

	SerializedValue SerializePanel(EditorPanel* Target);
	void DeSerializePanel(EditorPanel* Target, SerializedValue& Obj, PanelRegistry* Registry);
}