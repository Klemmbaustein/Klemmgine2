#ifdef EDITOR
#pragma once
#include "DropdownMenu.h"
#include "EditorIcons.h"
#include "EditorTheme.h"
#include "Panels/EditorPanel.h"
#include "StatusBar.kui.hpp"
#include <Engine/Objects/Reflection/ObjectReflection.h>
#include <kui/UI/UIBackground.h>
#include <Editor/FileAssetListProvider.h>

namespace engine::editor
{
	class EditorUI
	{
	public:

		static EditorPanel* FocusedPanel;

		static EditorUI* Instance;
		static kui::Font* EditorFont;
		static kui::Font* MonospaceFont;
		static EditorTheme Theme;
		EditorIcons ObjectIcons;

		kui::UIBox* DraggedBox = nullptr;
		kui::UIBox* MainBackground = nullptr;

		struct DraggedItem
		{
		public:
			string Name;
			string Type;
			string Path;
			string Icon;
			kui::Vec3f Color;
			bool IsAsset = false;
			bool Centered = true;
			ObjectTypeID ObjectType = 0;
		};

		DraggedItem CurrentDraggedItem;

		void StartAssetDrag(DraggedItem With);
		void StartDrag(kui::UIBox* bx, bool Centered);

		static std::pair<string, kui::Vec3f> GetExtIconAndColor(string Extension);

		enum class StatusType
		{
			Info,
			Warning,
			Error
		};

		template<typename T>
		static void ForEachPanel(std::function<void(T*)> Function)
		{
			if (!Instance || !Instance->RootPanel)
				return;
			Instance->RootPanel->ForEachPanel<T>(Function);
		}

		static void SetStatusMessage(string NewMessage, StatusType Type);

		EditorUI();
		~EditorUI();
		static string CreateAsset(string Path, string Name, string Extension);
		static void UpdateTheme(kui::Window* Target, bool Full);
		void Update();
		void ReloadUI();
		void UpdateBackgrounds();

		void AddMenuBarItem(string Name, std::vector<DropdownMenu::Option> Options);

		static string Asset(const string& name);
		AssetListProvider* AssetsProvider = new FileAssetListProvider();

	private:
		kui::UIBackground* Root = nullptr;
		kui::UIBackground* MenuBar = nullptr;
		kui::UIBackground* StatusBar = nullptr;

		EditorPanel* RootPanel = nullptr;
		StatusBarElement* StatsBarElement;

		string CurrentStatus;
		static void SetStatusMainThread(string NewMessage, StatusType Type);
	};
}
#endif