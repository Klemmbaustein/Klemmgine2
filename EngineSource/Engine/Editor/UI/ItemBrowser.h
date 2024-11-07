#pragma once
#include "EditorPanel.h"
#include <ItemBrowser.kui.hpp>
#include <kui/UI/UIScrollBox.h>

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
			bool Selected = false;
			std::function<void()> OnRightClick;
			std::function<void()> OnClick;
			string Image;
			kui::Vec3f Color;
		};

		std::vector<std::pair<Item, ItemBrowserButton*>> Buttons;
		kui::UIScrollBox* ItemsScrollBox = nullptr;

		void Update() override;
		void OnResized() override;

		void UpdateItems();
		virtual std::vector<Item> GetItems() = 0;
		virtual void Back() = 0;
	};
}