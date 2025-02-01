#ifdef EDITOR
#pragma once
#include <Core/Types.h>
#include <kui/Window.h>

namespace engine::editor
{
	class IPopupWindow
	{;
	protected:
		virtual void Update() = 0;
		virtual void Begin() = 0;
		virtual void Destroy() = 0;
		kui::Window* Popup = nullptr;

		void Open();

		void SetTitle(string NewTitle);

	public:
		IPopupWindow(string Name, kui::Vec2ui Size, bool Resizable, bool Closable);
		void Close();

		kui::Font* DefaultFont = nullptr;

	private:
		string Name;
		kui::Vec2ui Size;
		bool Resizable = false;
		bool ShouldClose = false;
		bool CanClose = false;
		void WindowThread(string Name, kui::Vec2ui Size);
	};
}
#endif