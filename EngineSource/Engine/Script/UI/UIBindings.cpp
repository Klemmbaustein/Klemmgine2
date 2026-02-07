#include "UIBindings.h"
#include <kui/KlemmUI.h>
#include <ds/native/nativeGeneric.hpp>
#include "ScriptUICanvas.h"
#include <ds/language.hpp>

using namespace engine;
using namespace engine::script::ui;
using namespace ds;

static void UIElement_new(InterpretContext* context)
{
	ds::ClassRef<ScriptUICanvas*> cls = context->popValue<RuntimeClass*>();

	cls.getValue() = new ScriptUICanvas(cls.classPtr);

	context->pushValue(cls.classPtr);
}

static void UIElement_empty(InterpretContext* context)
{
	ds::ClassRef<ScriptUICanvas*> cls = context->popValue<RuntimeClass*>();
}

static void UIElement_getChild(InterpretContext* context)
{
	auto type = GenericData(context);
	ds::ClassRef<ScriptUICanvas*> cls = context->popValue<RuntimeClass*>();

	auto text = context->popRuntimeString();

	auto classInstance = cls.getValue();
	auto box = classInstance->MarkupBox;

	context->pushValue(NativeModule::makePointerClass<kui::UIBox>(box->NamedChildren[text.ptr()]));
}

static void UIText_setText(InterpretContext* context)
{
	ds::ClassRef<kui::UIText*> cls = context->popValue<RuntimeClass*>();

	auto text = context->popRuntimeString();

	cls.getValue()->SetText(std::string(text.ptr(), text.length()));

	cls.classPtr->addRef();
	context->pushValue(cls);
}

UIBindings engine::script::ui::AddUIModule(ds::NativeModule& To, ds::LanguageContext* ToContext)
{
	auto StrType = ToContext->registry->getEntry<StringType>();

	auto UIBoxType = To.createClass<kui::UIBox*>("UIBox");

	auto UITextType = To.createClass<kui::UIText*>("UIText", UIBoxType);

	auto AddUIMethod = [&To](ClassType* type, NativeFunction fn) {
		fn.isDiscardable = true;

		To.addClassMethod(type, fn);
	};

	AddUIMethod(UITextType, NativeFunction({ FunctionArgument(StrType, "newText") },
		UITextType, "setText", &UIText_setText));

	auto ElementType = To.createClass<ScriptUICanvas*>("UIElement");

	ElementType->baseConstructor = To.addFunction(NativeFunction({}, nullptr, "UIElement.new", &UIElement_new));

	To.addClassMethod(ElementType,
		NativeGenericFunction({ FunctionArgument(StrType, "name") }, { GenericArgument("T", UIBoxType) },
			GenericArgumentType::getInstance(0, true), "getChild", &UIElement_getChild));

	To.addClassVirtualMethod(ElementType,
		NativeFunction({}, nullptr, "update", &UIElement_empty), 1);

	return UIBindings{
		.UITextType = UITextType
	};
}
