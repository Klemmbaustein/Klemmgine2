#ifdef EDITOR
#pragma once
#include "EditorPanel.h"
#include <ItemBrowser.kui.hpp>
#include <kui/UI/UIScrollBox.h>
#include <Engine/Objects/Reflection/ObjectReflection.h>

namespace engine::editor
{
	class ItemBrowser : public EditorPanel
	{
	public:
		string Path;
		string Filter;
		ItemBrowser(string Name, string InternalName);
		ItemBrowserHeading* Heading = nullptr;

		struct Item
		{
			string Name;
			string Path;
			bool Selected = false;
			bool IsDirectory = false;
			kui::Vec3f Color;
			std::function<void()> OnRightClick;
			std::function<void()> OnClick;
			string Image;
			ObjectTypeID Type = 0;
		};

		void Update() override;
		void OnResized() override;
		void UpdateItems();

		std::vector<Item*> GetSelected();

		enum class DisplayMode
		{
			Icons,
			List,
		};

		DisplayMode Mode = DisplayMode::Icons;

	protected:

		std::vector<std::pair<Item, kui::UIBox*>> Buttons;
		kui::UIScrollBox* ItemsScrollBox = nullptr;

		virtual string GetPathDisplayName() = 0;
		virtual void OnBackgroundRightClick(kui::Vec2f MousePosition) = 0;
		virtual void OnItemsRightClick(kui::Vec2f MousePosition);
		virtual std::vector<Item> GetItems() = 0;
		//virtual void OnItemDropped(EditorUI::Item);
		virtual void Back() = 0;
		void SetStatusText(string NewText);
	private:
		size_t LastItemsPerRow = 0;
		kui::UIText* StatusText;
		std::pair<Item, kui::UIBox*>* GetHoveredButton();
		std::vector<Item> CurrentItems;
		void DisplayList();
	};
}
#endif