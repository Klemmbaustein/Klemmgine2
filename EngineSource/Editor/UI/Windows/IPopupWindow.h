#ifdef EDITOR
#pragma once
#include <Core/Types.h>
#include <kui/Window.h>

namespace engine::editor
{
	/**
	 * @brief
	 A basic popup window put on top of the main window that blocks it until this window is done.
	 *
	 * It runs on a separate thread, and accessing main thread variables is dangerous from any functions of this class.
	 */
	class IPopupWindow
	{
	protected:

		/**
		 * @brief
		 * Called for each frame of the popup window's life.
		 */
		virtual void Update() = 0;

		/**
		 * @brief
		 * When the popup thread has started, this function is called to initialize the window on that thread.
		 */
		virtual void Begin() = 0;

		/**
		 * @brief
		 * Called when the popup has been closed.
		 */
		virtual void Destroy() = 0;

		virtual void OnThemeChanged();

		/**
		 * @brief
		 * When the popup thread has started, this function is called to initialize the window on that thread.
		 */
		kui::Window* Popup = nullptr;

		/**
		 * @brief
		 * Starts the popup thread and opens the window.
		 */
		void Open();

		void SetTitle(string NewTitle);

	public:

		/**
		 * @brief Constructs a popup.
		 * @param Title
		 * The title shown on the window's title bar.
		 * @param Size
		 * The size of the window in DPI-scaled pixels.
		 * @param Resizable
		 * Should the window be resizable.
		 * @param Resizable
		 * Should the window be closable.
		 *
		 * When overriding this constructor, call Open() to start the popup's thread
		 * after you're done ititializing it's variables from the main thread.
		 */
		IPopupWindow(string Name, kui::Vec2ui Size, bool Resizable, bool Closable);
		void Close();

		/**
		 * @brief The default font of this popup. Use this instead of the main thread's default font for thread safety.
		 */
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