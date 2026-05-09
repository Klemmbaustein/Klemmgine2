#pragma once
#include "DroppableBox.h"
#include <Engine/File/AssetRef.h>
#include <kui/UI/UITextField.h>
#include <kui/UI/UIBackground.h>
#include <kui/UI/UIScrollBox.h>
#include <Engine/Objects/Reflection/ObjectReflection.h>

namespace engine::editor
{
	class AssetSelector : public DroppableBox
	{
	public:
		AssetSelector(AssetRef InitialValue, kui::UISize Width, std::function<void()> OnChanged,
			bool EmptyIsDefault = false);
		AssetSelector(ObjectTypeID InitialType, ObjectTypeID SuperType, kui::UISize Width,
			std::function<void()> OnChanged, bool EmptyIsDefault = false);
		virtual ~AssetSelector() override;

		std::function<void()> OnChanged;

		bool EmptyIsDefault = false;

		AssetRef SelectedAsset;
		ObjectTypeID SelectedId = 0;
		ObjectTypeID SuperType = 0;
		Reflection::ObjectInfo Obj;
		kui::UIBackground* IconBackground = nullptr;
		kui::UITextField* AssetPath = nullptr;
		kui::UIScrollBox* SearchBackground = nullptr;
		kui::UIText* PathText = nullptr;
		void UpdateSelection();

		virtual void Tick() override;
	private:

		void Init(string Icon, kui::Vec3f Color, kui::UISize Width);
		string LastEnteredText;
		bool RemoveSearchList = false;
		bool ChangedText = false;
		bool IsAsset = false;
		void UpdateSearchSize();
		void RemoveSearchResults();
		void UpdateSearchResults();
		void UpdateSearchResultsAsset();
		void UpdateSearchResultsClass();
		void UpdateObjectClass();
	};
}