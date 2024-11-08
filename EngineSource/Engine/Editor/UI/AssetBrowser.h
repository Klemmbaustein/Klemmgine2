#pragma once
#include "ItemBrowser.h"

namespace engine::editor
{
	class AssetBrowser : public ItemBrowser
	{
	public:
		AssetBrowser();
		std::vector<Item> GetItems() override;
		void Back() override;
		void OnBackgroundRightClick(kui::Vec2f Position) override;
	};
}