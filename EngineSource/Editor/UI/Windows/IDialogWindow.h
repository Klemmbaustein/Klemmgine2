#pragma once
#include "IPopupWindow.h"
#include <kui/UI/UIBackground.h>
#include <functional>

namespace engine::editor
{
	/**
	 * @brief A dialog window, containing a background and buttons to interact with the dialog.
	 *
	 * Vaguely similar to the message boxes created by win32's MessageBox() functions.
	 * Like it's parent class IPopupWindow, it runs on a separate thread, and accessing main
	 * thread variables is dangerous.
	 */
	class IDialogWindow : public IPopupWindow
	{
	public:

		/**
		 * @brief An option for a dialog window.
		 */
		struct Option
		{
			/// The name of the option
			string Name;
			/// Will be run once the button is clicked.
			std::function<void()> OnClicked;
			/// Close the window after the option is clicked if true.
			bool Close = true;
			/// If true, this will be clicked automatically once enter is pressed.
			bool IsAccept = false;
			/// If true, this will be clicked automatically once escape is pressed.
			bool IsClose = false;
			/// Run OnClicked on the main thread. If false, it will be run on the same thread as the dialog window.
			bool OnMainThread = true;
		};

		/**
		 * @brief Constructs a dialog window.
		 * @param Title The title shown on the window's title bar.
		 * @param Options Any options the dialog should have.
		 * @param Size The size of the window in DPI-scaled pixels.
		 *
		 * Like with IPopupWindow, when overriding this constructor, call Open() to start the dialog window's thread
		 * after you're done initializing it's variables from the main thread.
		 */
		IDialogWindow(string Title, std::vector<Option> Options, kui::Vec2ui Size);
		virtual void Begin() override;

	protected:

		/**
		 * @brief
		 * Sets the buttons displayed on the popup to these new values.
		 * @param Options
		 * The new button options to display.
		 */
		void SetButtons(std::vector<Option> Options);

		/**
		 * @brief
		 * The background of this popup. Add any content here.
		 */
		kui::UIBox* Background = nullptr;

	private:

		void PressButton(const Option& o);

		std::vector<Option> Options;
		kui::UIBox* ButtonBackground = nullptr;
	};
}
