#include "PanelRegistry.h"

void engine::editor::PanelRegistry::RegisterPanel(string Name, string InternalName,
	std::function<EditorPanel* ()> CreatePanelFunction, bool IsVisible)
{
	this->Panels[InternalName] = PanelData{
		.CreatePanel = CreatePanelFunction,
		.InternalName = InternalName,
		.Name = Name,
		.IsVisible = IsVisible,
	};
}
