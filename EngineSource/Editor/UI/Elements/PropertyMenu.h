#pragma once
#include <kui/UI/UIScrollBox.h>
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
		void AddVecEntry(string Name, Vector3& Value, std::function<void()> OnChanged);
		void AddStringEntry(string Name, string& Value, std::function<void()> OnChanged);
		void AddBooleanEntry(string Name, bool& Value, std::function<void()> OnChanged);
		void AddIntEntry(string Name, int32& Value, std::function<void()> OnChanged);
		void AddFloatEntry(string Name, int32& Value, std::function<void()> OnChanged);
		void AddAssetRefEntry(string Name, AssetRef& Value, std::function<void()> OnChanged);
		void AddInfoEntry(string Name, string Value);

		void SetMode(Mode NewMode);

		void UpdateProperties();

	private:
		std::vector<std::function<void()>> UpdatePropertiesCallback;
	};
}