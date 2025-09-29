#pragma once
#include <Editor/UI/Elements/PropertyMenu.h>
#include <Core/Types.h>

namespace engine::editor
{
	class SettingsPage
	{
	public:
		virtual ~SettingsPage() = default;

		virtual void Generate(PropertyMenu* Target) = 0;

		string Name;
	};
}