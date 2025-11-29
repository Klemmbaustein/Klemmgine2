#pragma once
#include <kui/UI/UIScrollBox.h>
#include <kui/UI/UIDropdown.h>
#include <PropertyPanel.kui.hpp>
#include <Core/Types.h>
#include <Core/Vector.h>
#include <Editor/UI/EditorUI.h>
#include <Engine/File/AssetRef.h>

namespace engine::editor
{
	class PropertyMenu : public kui::UIScrollBox
	{
	public:

		enum class Mode
		{
			DisplayText,
			DisplayEntries,
		};

		PropertyMenu(kui::Font* MenuFont = EditorUI::EditorFont);

		void OnAttached() override;

		void Update() override;
		void Clear();

		bool IsCompact = false;
		kui::UISize ElementSize = 0;

		PropertyHeaderElement* CreateNewHeading(string Title, bool HasPadding = true);

		PropertyEntryElement* CreateNewEntry(string Name);
		void AddVecEntry(string Name, Vector3& Value, std::function<void()> OnChanged, bool IsColor = false);
		void AddStringEntry(string Name, string& Value, std::function<void()> OnChanged);
		void AddBooleanEntry(string Name, bool& Value, std::function<void()> OnChanged);
		void AddButtonEntry(string Name, string Label, std::function<void()> OnClicked);
		void AddButtonsEntry(string Name, std::vector<std::pair<string, std::function<void()>>> Buttons);
		void AddIntEntry(string Name, int32& Value, std::function<void()> OnChanged);
		void AddFloatEntry(string Name, float& Value, std::function<void()> OnChanged);
		void AddAssetRefEntry(string Name, AssetRef& Value, std::function<void()> OnChanged);
		void AddDropdownEntry(string Name,
			std::vector<kui::UIDropdown::Option> Values,
			std::function<void(kui::UIDropdown::Option)> OnChanged,
			size_t DefaultIndex);
		void AddInfoEntry(string Name, string Value);

		void SetMode(Mode NewMode);

		void UpdateProperties();

	private:
		kui::Font* MenuFont = nullptr;
		std::vector<std::function<void()>> UpdatePropertiesCallback;
	};
}