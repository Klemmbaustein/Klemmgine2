#ifdef EDITOR
#include "IPopupWindow.h"
#include <kui/UI/UIBackground.h>
#include <functional>

namespace engine::editor
{
	class IDialogWindow : public IPopupWindow
	{
	public:
		struct Option
		{
			string Name;
			std::function<void()> OnClicked;
			bool Close = true;
			bool OnMainThread = true;
		};

		IDialogWindow(string Title, std::vector<Option> Options, kui::Vec2ui Size);


		virtual void Begin() override;
		
	protected:

		void SetButtons(std::vector<Option> Options);

		kui::UIBox* Background = nullptr;

	private:
		std::vector<Option> Options;
		kui::UIBox* ButtonBackground = nullptr;
	};
}
#endif