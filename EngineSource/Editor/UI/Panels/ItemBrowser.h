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

	protected:

		std::vector<std::pair<Item, ItemBrowserButton*>> Buttons;
		kui::UIScrollBox* ItemsScrollBox = nullptr;

		virtual string GetPathDisplayName() = 0;
		virtual void OnBackgroundRightClick(kui::Vec2f MousePosition) = 0;
		virtual std::vector<Item> GetItems() = 0;
		//virtual void OnItemDopped(EditorUI::Item);
		virtual void Back() = 0;
		void SetStatusText(string NewText);
	private:
		kui::UIText* StatusText;
		std::pair<Item, ItemBrowserButton*>* GetHoveredButton();
		std::vector<Item> CurrentItems;
		void DisplayList();
	};
}
#endif