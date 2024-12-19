#ifdef EDITOR
#pragma once
#include <Engine/Types.h>
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

		float WidthFraction = 0.5f;

		bool Visible = true;

		void AddChild(EditorPanel* NewChild, Align ChildAlign, bool Select = false);

		void GenerateTabs();
		size_t SelectedTab = 0;

		void SetFocused();

	protected:
		bool CanClose = true;

	private:
		void UpdateFocusState();
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