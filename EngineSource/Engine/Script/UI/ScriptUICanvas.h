#include <Engine/UI/UICanvas.h>
#include <ds/class.hpp>
#include <kui/DynamicMarkup.h>

namespace engine::script::ui
{
	class ScriptUICanvas : public UICanvas
	{
	public:

		ScriptUICanvas(ds::RuntimeClass* ScriptObject);

		~ScriptUICanvas() override;

		void Update() override;

		kui::markup::UIDynMarkupBox* MarkupBox = nullptr;
		ds::RuntimeClass* ScriptObject = nullptr;
	};
}