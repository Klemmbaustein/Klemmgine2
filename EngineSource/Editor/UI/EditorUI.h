#pragma once
#include "DropdownMenu.h"
#include "EditorIcons.h"
#include "EditorTheme.h"
#include "Panels/PanelRegistry.h"
#include "Panels/EditorPanel.h"
#include "StatusBar.kui.hpp"
#include <Engine/Objects/Reflection/ObjectReflection.h>
#include <kui/UI/UIBackground.h>
#include <Editor/FileAssetListProvider.h>

namespace engine::editor
{
	/**
	 * @brief
	 * The main editor UI class.
	 */
	class EditorUI
	{
	public:

		/**
		 * @brief
		 * The panel currently focused by the user.
		 *
		 * This is usually the panel that was last clicked on or opened.
		 */
		static EditorPanel* FocusedPanel;

		/**
		 * @brief
		 * The current editor instance.
		 */
		static EditorUI* Instance;
		/**
		 * @brief
		 * The font used by the editor.
		 *
		 * For safety, do not use this in any other window on other threads.
		 */
		static kui::Font* EditorFont;
		/**
		 * @brief
		 * The monospace font used by the editor in logs and text editors.
		 *
		 * For safety, do not use this in any other window on other threads.
		 */
		static kui::Font* MonospaceFont;

		/**
		 * @brief
		 * The currently active theme.
		 *
		 * It's safe to access this from any thread.
		 */
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

		/**
		 * @brief
		 * Gets the icon path and color for an extension.
		 * @param Extension
		 * The extension to check
		 * @return
		 * A pair with the first item being a full path to the icon to use and
		 * the second item being the file type's associated color.
		 */
		static std::pair<string, kui::Vec3f> GetExtIconAndColor(string Extension);

		enum class StatusType
		{
			Info,
			Warning,
			Error
		};

		/**
		 * @brief
		 * Runs a function for each editor panel of type T
		 * @tparam T
		 * The type of the panels to iterate.
		 * @param Function
		 * The function to call for each panel matching the type T.
		 */
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

		/**
		 * @brief
		 * Creates an asset at the given path and renames it if another conflicting
		 * item already exists at the location.
		 * @param Path
		 * The path where the asset should be created.
		 * @param Name
		 * The name of the asset
		 * @param Extension
		 * The extension the asset should have.
		 * @return
		 * The actual path at which the asset has been created.
		 */
		static string CreateAsset(string Path, string Name, string Extension);

		/**
		 * @brief
		 * Creates a directory at the given path and renames it if another conflicting
		 * item already exists at the location.
		 * @param Path
		 * The path where the directory should be created.
		 * @return
		 * The actual path at which the directory has been created.
		 */
		static string CreateDirectory(string Path);
		static void UpdateTheme(kui::Window* Target, bool Full);
		void Update();
		void UpdateBackgrounds();

		/**
		 * @brief
		 * Initializes the theme variables by loading theme files.
		 */
		static void InitTheme();

		/**
		 * @brief
		 * Adds a new category item to the menu bar.
		 * @param Name
		 * The Path of the category.
		 * @param Options
		 * Any options in the category.
		 */
		void AddMenuBarItem(string Name, std::vector<DropdownMenu::Option> Options);

		/**
		 * @brief
		 * Converts an editor asset path into a full path.
		 * @param Path
		 * The path of the editor asset relative to the Editor/Assets/ folder.
		 * @return
		 * The full path of the editor asset.
		 */
		static string Asset(const string& Path);
		AssetListProvider* AssetsProvider = new FileAssetListProvider();

		static string GetLayoutConfigPath();

		PanelRegistry Panels;

	private:

		void RegisterDefaultPanels();

		void LoadEditorStateConfig();
		void SaveEditorStateConfig();

		std::vector<DropdownMenu::Option> GetPanelMenuOptions();

		kui::UIBackground* Root = nullptr;
		kui::UIBackground* MenuBar = nullptr;
		kui::UIBackground* StatusBar = nullptr;

		EditorPanel* RootPanel = nullptr;
		StatusBarElement* StatsBarElement;

		string CurrentStatus;
		static void SetStatusMainThread(string NewMessage, StatusType Type);
	};
}
