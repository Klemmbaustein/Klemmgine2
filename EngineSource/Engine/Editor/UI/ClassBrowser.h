#pragma once
#include "ItemBrowser.h"

namespace engine::editor
{
	class ClassBrowser : public ItemBrowser
	{
	public:
		ClassBrowser();

		virtual void OnBackgroundRightClick(kui::Vec2f At) override;
		virtual std::vector<Item> GetItems() override;
		virtual void Back() override;
	};
}