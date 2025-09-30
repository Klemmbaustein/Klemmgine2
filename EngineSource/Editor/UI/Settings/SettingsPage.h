#pragma once
#include <Editor/UI/Elements/PropertyMenu.h>
#include <Core/Types.h>

namespace engine::editor
{
	class SettingsWindow;

	class SettingsPage
	{
	public:
		virtual ~SettingsPage() = default;

		virtual void Generate(PropertyMenu* Target, SettingsWindow* TargetWindow) = 0;

		string Name;
	};
}