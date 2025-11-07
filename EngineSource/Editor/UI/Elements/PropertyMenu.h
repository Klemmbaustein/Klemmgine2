#pragma once
#include <kui/UI/UIScrollBox.h>
#include <kui/UI/UIDropdown.h>
#include <PropertyPanel.kui.hpp>
#include <Core/Types.h>
#include <Core/Vector.h>
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

		PropertyMenu();

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
		std::vector<std::function<void()>> UpdatePropertiesCallback;
	};
}