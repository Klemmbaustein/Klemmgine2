#include "IPopupWindow.h"
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
		};

		IDialogWindow(string Title, std::vector<Option> Options, kui::Vec2ui Size);

		void Begin();
		void Update();
		void Destroy();
	private:
		std::vector<Option> Options;
	};
}