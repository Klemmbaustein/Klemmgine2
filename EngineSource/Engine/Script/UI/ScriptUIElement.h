#include <Engine/UI/UICanvas.h>
#include <ds/class.hpp>
#include <kui/DynamicMarkup.h>

namespace engine::script::ui
{
	class ScriptUIElement : public kui::markup::UIDynMarkupBox
	{
	public:

		ScriptUIElement(ds::RuntimeClass* ScriptObject);

		~ScriptUIElement() override;

		ds::RuntimeClass* ScriptObject = nullptr;
	};
}