#ifdef EDITOR
#pragma once
#include "DroppableBox.h"
#include <Engine/File/AssetRef.h>
#include <kui/UI/UITextField.h>
#include <kui/UI/UIBackground.h>
#include <kui/UI/UIScrollBox.h>

namespace engine::editor
{
	class AssetSelector : public DroppableBox
	{
	public:
		AssetSelector(AssetRef InitialValue, float Width, std::function<void()> OnChanged);
		virtual ~AssetSelector() override;

		std::function<void()> OnChanged;

		AssetRef SelectedAsset;
		kui::UIBackground* IconBackground = nullptr;
		kui::UITextField* AssetPath = nullptr;
		kui::UIScrollBox* SearchBackground = nullptr;
		kui::UIText* PathText = nullptr;

		virtual void Tick() override;
	private:
		string LastEnteredText;
		bool RemoveSearchList = false;
		void UpdateSelection();
		void UpdateSearchSize();
		void RemoveSearchResults();
		void UpdateSearchResults();
	};
}
#endif