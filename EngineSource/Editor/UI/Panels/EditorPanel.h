#pragma once
#include <Core/Types.h>
#include <kui/Vec2.h>
#include <EditorPanel.kui.hpp>
#include <kui/Input.h>
#include <optional>

namespace engine::editor
{
	class EditorPanel
	{
	public:
		/**
		 * @brief
		 * Creates a new editor panel.
		 * @param Name
		 * The name of the panel displayed to the user. This name can change
		 * (for unsaved-indicators or some sort of status)
		 * @param InternalName
		 * The internal name, used for serialization.
		 * This name can't change and uniquely identifies this type of panel.
		 * "panel" is the default ID and means this is a positional panel.
		 */
		EditorPanel(string Name, string InternalName = "panel");
		virtual ~EditorPanel();
		/**
		 * @brief
		 * The size of this panel.
		 */
		kui::Vec2f Size;
		/**
		 * @brief
		 * The position of the panel.
		 */
		kui::Vec2f PanelPosition;
		/**
		 * @brief
		 * The parent panel of this panel.
		 */
		EditorPanel* Parent = nullptr;
		/**
		 * @brief
		 * The children panels of this panel.
		 *
		 * If this panel has children, it itself won't be visible.
		 */
		std::vector<EditorPanel*> Children;
		EditorPanelElement* PanelElement = nullptr;

		/**
		 * @brief
		 * The background element of this panel, in which content is shown.
		 */
		kui::UIBackground* Background = nullptr;

		void UpdateLayout();

		void UpdatePanel();

		virtual void OnResized();
		virtual void Update();

		enum class Align
		{
			/// Align child panels horizontally.
			Horizontal,
			/// Align child panels vertically.
			Vertical,
			/// Align child panels as tabs.
			Tabs
		};

		/**
		 * @brief
		 * Executes the given function for each panel in the panel tree with the given type.
		 * @tparam T
		 * The type to iterate
		 * @param Func
		 * The function to call.
		 */
		template<typename T>
		void ForEachPanel(std::function<void(T*)> Func)
		{
			if (dynamic_cast<T*>(this))
			{
				Func(dynamic_cast<T*>(this));
			}
			for (auto& i : Children)
			{
				i->ForEachPanel<T>(Func);
			}
		}

		/**
		 * @brief
		 * Sets the size of this panel in screen relative coordinates.
		 * @param NewWidth
		 * The size of this panel in screen relative coordinates.
		 * @return
		 * This panel, for chaining methods.
		 */
		EditorPanel* SetWidth(float NewWidth);

		/**
		 * @brief
		 * The width of this panel in screen
		 */
		float SizeFraction = 0.5f;
		bool ShouldUpdate = false;

		static void UpdateAllPanels();

		void AddChild(EditorPanel* NewChild, Align ChildAlign, bool Select = false, size_t Position = SIZE_MAX);

		void GenerateTabs();

		void SetFocused();
		void SetName(string NewName);

		/**
		 * @brief
		 * Function called when the editor theme colors have changed.
		 */
		virtual void OnThemeChanged();

		/**
		 * @brief
		 * Gets the index of a child panel in the @ref Children vector.
		 * @param Child
		 * The child to look for.
		 * @return
		 * The found index, or SIZE_MAX if it wasn't found.
		 */
		size_t IndexOf(EditorPanel* Child) const;
		Align ChildrenAlign = Align::Horizontal;

		enum class ShortcutOptions
		{
			/// No options.
			None = 0,
			/// Allow the shortcut to activate while focus is in a text box.
			AllowInText = 0b01,
			/// The shortcut is global and doesn't require the panel to be focused.
			Global = 0b10,
		};

		/**
		 * @brief
		 * Creates a keyboard shortcut for this panel.
		 *
		 * The shortcut
		 * @param NewKey
		 * The key that the shortcut listens to. When it is pressed, the shortcut activates.
		 * @param Modifier
		 * A modifier for the shortcut that also needs to be pressed when the shortcut is activated.
		 * Usually Shift or Ctrl.
		 * @param OnPressed
		 * The function to call once the shortcut was triggered.
		 * @param Options
		 * Options for the shortcut.
		 */
		void AddShortcut(kui::Key NewKey, std::optional<kui::Key> Modifier, std::function<void()> OnPressed,
			ShortcutOptions Options = ShortcutOptions::None);

	protected:

		bool CanClose = true;
		bool Visible = true;
		size_t SelectedTab = 0;

		static EditorPanel* DraggedPanel;
		static bool DraggingHorizontal;
		static float DragStartPosition;

		/**
		 * @brief
		 * Function called when the panel is closed.
		 *
		 * This function can cancel the close operation.
		 * For example when the panel contains unsaved changes, a prompt can be shown instead of completing the close.
		 *
		 * @return
		 * True if the panel should continue the close operation, false if not.
		 */
		virtual bool OnClosed();

		struct MoveOperation
		{
			kui::UIBackground* HighlightBackground = nullptr;
			EditorPanel* Panel = nullptr;
			EditorPanel* EndTarget = nullptr;
			size_t TabPosition = 0;
			Align TabAlign = Align::Tabs;

			enum Type
			{
				Up,
				Down,
				Left,
				Right,
				Center
			};
			Type MoveType = Center;
		};

		static MoveOperation Move;

	private:
		void ClearParent();
		void HandleResizing();
		void HandleResizeDrag();
		void UpdateFocusState();
		void MovePanel();
		void UpdatePanelMove();
		void AddTabFor(EditorPanel* Target, bool Selected);
		std::vector<EditorPanelTab*> TabElements;
		string Name, TypeName;

		std::vector<kui::Key> Shortcuts;

		kui::Vec2f UsedSize;
		kui::Vec2f Position;
		kui::Vec2f OldUsedSize;
		kui::Vec2f OldPosition;
		kui::Vec2f UsedSizeToPanelSize(kui::Vec2f Used);
		kui::Vec2f PositionToPanelPosition(kui::Vec2f Pos);
	};
}