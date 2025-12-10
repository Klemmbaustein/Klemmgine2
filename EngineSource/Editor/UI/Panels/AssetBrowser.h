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
		void OnItemsRightClick(kui::Vec2f Position) override;

		string GetPathDisplayName() override;

	private:
		void DuplicateFile(string FilePath);
		void RenameFile(string FilePath);
	};
}
