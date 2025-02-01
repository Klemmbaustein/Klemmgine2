#ifdef EDITOR
#pragma once
#include <Core/Types.h>
#include <kui/Vec2.h>
#include <EditorPanel.kui.hpp>

namespace engine::editor
{
	class EditorPanel
	{
	public:
		EditorPanel(string Name, string InternalName = "panel");
		virtual ~EditorPanel();
		kui::Vec2f Size;
		kui::Vec2f PanelPosition;
		EditorPanel* Parent = nullptr;
		std::vector<EditorPanel*> Children;
		EditorPanelElement* PanelElement = nullptr;

		kui::UIBackground* Background = nullptr;

		void UpdateLayout();

		void UpdatePanel();
		bool ShouldUpdate = false;

		virtual void OnResized();
		virtual void Update();

		enum class Align
		{
			Horizontal,
			Vertical,
			Tabs
		};

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

		EditorPanel* SetWidth(float NewWidth);

		float SizeFraction = 0.5f;

		static void UpdateAllPanels();

		bool Visible = true;

		void AddChild(EditorPanel* NewChild, Align ChildAlign, bool Select = false, size_t Position = SIZE_MAX);

		void GenerateTabs();
		size_t SelectedTab = 0;

		void SetFocused();
		void SetName(string NewName);

	protected:
		bool CanClose = true;

		static EditorPanel* DraggedPanel;
		static bool DraggingHorizontal;
		static float DragStartPosition;
		
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
		Align ChildrenAlign = Align::Horizontal;
		
		kui::Vec2f UsedSize;
		kui::Vec2f Position;
		kui::Vec2f UsedSizeToPanelSize(kui::Vec2f Used);
		kui::Vec2f PositionToPanelPosition(kui::Vec2f Pos);
	};
}
#endif