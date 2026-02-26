#pragma once
#include <Core/Types.h>
#include <map>
#include <functional>

namespace engine::editor
{
	class EditorPanel;

	class PanelRegistry
	{
	public:

		void RegisterPanel(string Name, string InternalName, std::function<EditorPanel* ()> CreatePanelFunction,
			bool IsVisible);

		struct PanelData
		{
			std::function<EditorPanel* ()> CreatePanel;
			string InternalName;
			string Name;
			bool IsVisible = true;
		};

		std::map<string, PanelData> Panels;
	};
}