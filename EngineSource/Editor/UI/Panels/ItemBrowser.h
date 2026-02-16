#pragma once
#include "EditorPanel.h"
#include <ItemBrowser.kui.hpp>
#include <kui/UI/UIScrollBox.h>
#include <Engine/Objects/Reflection/ObjectReflection.h>

namespace engine::editor
{
	/**
	 * @brief
	 * Base class for a panel displaying a list of items (assets, classes, etc.)
	 */
	class ItemBrowser : public EditorPanel
	{
	public:
		/// The current path that the item browser has open.
		string Path;
		/// The search query that item names are filtered for.
		string Filter;
		/**
		 * @brief
		 * Creates an item browser
		 * @param Name
		 * The name of the panel displayed to the user. This name can change
		 * (for unsaved-indicators or some sort of status)
		 * @param InternalName
		 * The internal name, used for serialization.
		 * This name can't change and uniquely identifies this type of panel.
		 */
		ItemBrowser(string Name, string InternalName);

		/**
		 * @brief
		 * An item in the item browser.
		 */
		struct Item
		{
			/**
			 * @brief
			 * The name of the item displayed to the user.
			 */
			string Name;
			/**
			 * @brief
			 * The full path of the item, for internal use.
			 */
			string Path;
			/**
			 * @brief
			 * True if the item is selected, false if not.
			 */
			bool Selected = false;
			/**
			 * @brief
			 * True if the item is a directory.
			 *
			 * Directories are sorted before everything else.
			 */
			bool IsDirectory = false;
			/**
			 * @brief
			 * The color to display for the item, used as a background for the item.
			 */
			kui::Vec3f Color;
			/**
			 * @brief
			 * A function called when the item is clicked with the right mouse button.
			 *
			 * Usually shows a dropdown.
			 */
			std::function<void()> OnRightClick;
			/**
			 * @brief
			 * A function called when the item is clicked or otherwise opened.
			 */
			std::function<void()> OnClick;
			/**
			 * @brief
			 * A path to the image file that the item should show.
			 */
			string Image;
			/**
			 * @brief
			 * A type ID, deciding what object to create when it is dragged into the viewport.
			 */
			ObjectTypeID Type = 0;
		};

		void Update() override;
		void OnResized() override;
		void OnThemeChanged() override;

		SerializedValue Serialize() override;
		void DeSerialize(SerializedValue* From) override;

		/**
		 * @brief
		 * Refreshes the items shown in the panel.
		 */
		void UpdateItems();

		/**
		 * @brief
		 * Returns a list of all items that have been selected.
		 * @return
		 * A list of items in the browsers where Selected = true.
		 */
		std::vector<Item*> GetSelected();

		/**
		 * @brief
		 * A display mode for an item browser.
		 */
		enum class DisplayMode
		{
			/// A grid of larger icons with their names below. Like Windows' File Explorer's "Large icons" view.
			Icons,
			/// A list of names with smaller icons next to the name. Like Windows' File Explorer's "List" view.
			List,
			Tree,
		};

		/// The display mode to use for the next update.
		DisplayMode Mode = DisplayMode::Icons;

	protected:

		/**
		 * @brief
		 * The buttons currently active.
		 */
		std::vector<std::pair<Item, kui::UIBox*>> Buttons;

		/**
		 * @brief
		 * Gets the display name of the current path.
		 *
		 * @see Path
		 * @return
		 * A user-friendly version of Path.
		 */
		virtual string GetPathDisplayName() = 0;

		/**
		 * @brief
		 * Function called when the background is right-clicked.
		 * @param MousePosition
		 * The position of the mouse cursor on the right click.
		 */
		virtual void OnBackgroundRightClick(kui::Vec2f MousePosition) = 0;

		/**
		 * @brief
		 * Function called when multiple items are right-clicked at the same time.
		 *
		 * @see GetSelected()
		 *
		 * @param MousePosition
		 * The position of the mouse cursor on the right click.
		 */
		virtual void OnItemsRightClick(kui::Vec2f MousePosition);

		/**
		 * @brief
		 * Gets the items the browser should display.
		 *
		 * @see Item
		 * @return
		 * A list of items that are displayed. They will possibly be filtered after (@see Filter)
		 */
		virtual std::vector<Item> GetItems(string Path) = 0;

		/**
		 * @brief
		 * Navigates back one level, like moving back to the parent folder.
		 */
		virtual void Back() = 0;
		kui::UIScrollBox* ItemsScrollBox = nullptr;
		std::set<string> ExpandedItems;
	private:
		void SetStatusText(string NewText);
		void UpdateStatusText(size_t NumItems);
		ItemBrowserHeading* Heading = nullptr;
		size_t LastItemsPerRow = 0;
		kui::UIText* StatusText;
		std::pair<Item, kui::UIBox*>* GetHoveredButton();
		std::vector<Item> CurrentItems;
		void DisplayItems();
		void DisplayTree(string Path, const std::vector<Item>& Items, size_t Depth, size_t& Index);

		void OnButtonClicked(size_t i, kui::UIBox* Element, bool IsList);
	};
}