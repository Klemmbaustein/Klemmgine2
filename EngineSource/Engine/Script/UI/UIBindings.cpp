#include "ScriptUICanvas.h"
#include "ScriptUIElement.h"
#include "UIBindings.h"
#include <ds/callableWrapper.hpp>
#include <ds/language.hpp>
#include <ds/native/nativeGeneric.hpp>
#include <ds/parser/types/functionType.hpp>
#include <Engine/Script/ScriptSubsystem.h>
#include <kui/KlemmUI.h>

using namespace engine;
using namespace engine::script::ui;
using namespace engine::script;
using namespace ds;
using namespace kui;

static void UIScriptCanvas_new(InterpretContext* context)
{
	ClassRef<ScriptUICanvas*> cls = context->popValue<RuntimeClass*>();

	cls.getValue() = new ScriptUICanvas(cls.classPtr);

	context->pushValue(cls.classPtr);
}

static void UIScriptCanvas_empty(InterpretContext* context)
{
	ClassRef<ScriptUICanvas*> cls = context->popValue<RuntimeClass*>();
}

static void UIScriptCanvas_getChild(InterpretContext* context)
{
	auto type = GenericData(context);
	ClassRef<ScriptUICanvas*> cls = context->popValue<RuntimeClass*>();

	auto text = context->popRuntimeString();

	auto classInstance = cls.getValue();
	auto box = classInstance->MarkupBox;

	auto found = box->NamedChildren[text.ptr()];

	if (ScriptSubsystem::Instance->UIObjectMappings.contains(found))
	{
		context->pushValue(ScriptSubsystem::Instance->UIObjectMappings[found]);
		return;
	}

	context->pushValue(NativeModule::makePointerClass<UIBox>(found));
}

static void UIScriptElement_new(InterpretContext* context)
{
	ClassRef<ScriptUIElement*> cls = context->popValue<RuntimeClass*>();

	cls.getValue() = new ScriptUIElement(cls.classPtr);

	context->pushValue(cls.classPtr);
}

static void UIScriptElement_getChild(InterpretContext* context)
{
	auto type = GenericData(context);
	ClassRef<ScriptUIElement*> cls = context->popValue<RuntimeClass*>();

	auto text = context->popRuntimeString();

	auto classInstance = cls.getValue();

	context->pushValue(NativeModule::makePointerClass<UIBox>(classInstance->NamedChildren[text.ptr()]));
}

static void UIBox_new(InterpretContext* context)
{
	ClassRef<UIBox*> cls = context->popValue<RuntimeClass*>();

	Bool horizontal = context->popValue<Bool>();

	cls.getValue() = new UIBox(horizontal);

	context->pushValue(cls);
}

static void UIBox_getUsedSize(InterpretContext* context)
{
	ds::ClassRef<UIBox*> cls = context->popValue<RuntimeClass*>();

	context->pushValue(cls.getValue()->GetUsedSize());
}

static void UIBox_addChild(InterpretContext* context)
{
	ClassRef<UIBox*> cls = context->popValue<RuntimeClass*>();
	ClassPtr<UIBox*> child = context->popPtr<UIBox*>();

	cls.getValue()->AddChild(*child.get());

	cls.classPtr->addRef();
	context->pushValue(cls);
}

static void UIBox_setMinSize(InterpretContext* context)
{
	ClassRef<UIBox*> cls = context->popValue<RuntimeClass*>();
	SizeVec size = context->popValue<SizeVec>();

	cls.getValue()->SetMinSize(size);

	cls.classPtr->addRef();
	context->pushValue(cls);
}

static void UIBox_setMaxSize(InterpretContext* context)
{
	ClassRef<UIBox*> cls = context->popValue<RuntimeClass*>();
	SizeVec size = context->popValue<SizeVec>();

	cls.getValue()->SetMaxSize(size);

	cls.classPtr->addRef();
	context->pushValue(cls);
}

static void UIBox_setPadding(InterpretContext* context)
{
	ClassRef<UIBox*> cls = context->popValue<RuntimeClass*>();
	UISize down = context->popValue<UISize>();
	UISize up = context->popValue<UISize>();
	UISize right = context->popValue<UISize>();
	UISize left = context->popValue<UISize>();

	cls.getValue()->SetPadding(up, down, left, right);

	cls.classPtr->addRef();
	context->pushValue(cls);
}

static void UIBox_setAllPadding(InterpretContext* context)
{
	ClassRef<UIBox*> cls = context->popValue<RuntimeClass*>();
	UISize allPadding = context->popValue<UISize>();

	cls.getValue()->SetPadding(allPadding);

	cls.classPtr->addRef();
	context->pushValue(cls);
}

static void UIBackground_new(InterpretContext* context)
{
	ClassRef<UIBackground*> cls = context->popValue<RuntimeClass*>();
	Vec3f Color = context->popValue<Vec3f>();
	Bool Horizontal = context->popValue<Bool>();

	cls.getValue() = new UIBackground(Horizontal, 0, Color);

	context->pushValue(cls);
}

static void UIBackground_setColor(InterpretContext* context)
{
	ClassRef<UIBackground*> cls = context->popValue<RuntimeClass*>();
	Vec3f Color = context->popValue<Vec3f>();

	cls.getValue()->SetColor(Color);

	cls.classPtr->addRef();
	context->pushValue(cls);
}

static void UIBackground_setBorder(InterpretContext* context)
{
	ClassRef<UIBackground*> cls = context->popValue<RuntimeClass*>();
	Vec3f Color = context->popValue<Vec3f>();
	UISize Size = context->popValue<UISize>();

	cls.getValue()->SetBorder(Size, Color);

	cls.classPtr->addRef();
	context->pushValue(cls);
}

static void UIButton_new(InterpretContext* context)
{
	ClassRef<UIButton*> cls = context->popValue<RuntimeClass*>();
	CallableWrapper<void> OnClicked = { context->popValue<RuntimeClass*>(), context };
	Vec3f Color = context->popValue<Vec3f>();
	Bool Horizontal = context->popValue<Bool>();

	cls.getValue() = new UIButton(Horizontal, 0, Color, OnClicked);

	context->pushValue(cls);
}

static void UIButton_setOnClicked(InterpretContext* context)
{
	ClassRef<UIButton*> cls = context->popValue<RuntimeClass*>();
	CallableWrapper<void> OnClicked = { context->popValue<RuntimeClass*>(), context };

	cls.getValue()->OnClicked = OnClicked;

	cls.classPtr->addRef();
	context->pushValue(cls);
}

static void UIText_new(InterpretContext* context)
{
	ClassRef<UIText*> cls = context->popValue<RuntimeClass*>();
	RuntimeStr Text = context->popRuntimeString();
	Vec3f Color = context->popValue<Vec3f>();
	UISize TextSize = context->popValue<UISize>();

	cls.getValue() = new UIText(TextSize, Color, string(Text.ptr(), Text.length()), VideoSubsystem::Current->DefaultFont);

	context->pushValue(cls);
}

static void UIText_setText(InterpretContext* context)
{
	ClassRef<UIText*> cls = context->popValue<RuntimeClass*>();

	auto text = context->popRuntimeString();

	cls.getValue()->SetText(std::string(text.ptr(), text.length()));

	cls.classPtr->addRef();
	context->pushValue(cls);
}

static void UIText_setColor(InterpretContext* context)
{
	ClassRef<UIText*> cls = context->popValue<RuntimeClass*>();

	auto text = context->popValue<Vec3f>();

	cls.getValue()->SetColor(text);

	cls.classPtr->addRef();
	context->pushValue(cls);
}

static void UISize_new_screen(InterpretContext* context)
{
	context->pushValue(UISize(context->popValue<Float>()));
}

static void UISize_new(InterpretContext* context)
{
	SizeMode s = context->popValue<SizeMode>();
	Float f = context->popValue<Float>();

	context->pushValue(UISize(f, s));
}

static void UISizeVec_new(InterpretContext* context)
{
	UISize y = context->popValue<UISize>();
	UISize x = context->popValue<UISize>();

	context->pushValue(SizeVec(x, y));
}

static void UISizeVec_new_same(InterpretContext* context)
{
	UISize xy = context->popValue<UISize>();

	context->pushValue(UISize(xy));
}

static void UISize_pixels(InterpretContext* context)
{
	context->pushValue(context->popValue<UISize>().GetPixels());
}

static void UI_pixels(InterpretContext* context)
{
	context->pushValue(UISize::Pixels(context->popValue<Float>()));
}

UIBindings engine::script::ui::AddUIModule(ds::NativeModule& To, ds::NativeModule& BaseModule,
	ds::LanguageContext* ToContext)
{
	auto StrType = ToContext->registry->getEntry<StringType>();
	auto FloatInst = ToContext->registry->getEntry<FloatType>();
	auto BoolInst = ToContext->registry->getEntry<BoolType>();

	auto Vec3Type = BaseModule.getType("Vector3");
	auto Vec2Type = BaseModule.getType("Vector2");

	auto SizeModeType = To.createEnum("SizeMode");

	To.addEnumValue(SizeModeType, "screenRelative", SizeMode::ScreenRelative);
	To.addEnumValue(SizeModeType, "aspectRelative", SizeMode::AspectRelative);
	To.addEnumValue(SizeModeType, "pixelRelative", SizeMode::PixelRelative);
	To.addEnumValue(SizeModeType, "parentRelative", SizeMode::ParentRelative);

	auto SizeType = DS_CREATE_STRUCT(UISize);
	DS_STRUCT_MEMBER_NAME(SizeType, UISize, Value, value, FloatInst);
	DS_STRUCT_MEMBER_NAME(SizeType, UISize, Mode, mode, SizeModeType);

	To.addClassConstructor(SizeType, NativeFunction({ FunctionArgument(FloatInst, "value") },
		nullptr, "UISize.new.screen", &UISize_new_screen));
	To.addClassConstructor(SizeType, NativeFunction({ FunctionArgument(FloatInst, "value"), FunctionArgument(SizeModeType, "mode") },
		nullptr, "UISize.new", &UISize_new));
	To.addType(SizeType);

	auto SizeVecType = new NativeStructType(sizeof(SizeVec), "UISizeVec");
	DS_STRUCT_MEMBER_NAME(SizeVecType, SizeVec, X, x, SizeType);
	DS_STRUCT_MEMBER_NAME(SizeVecType, SizeVec, Y, y, SizeType);

	To.addClassConstructor(SizeVecType, NativeFunction({ FunctionArgument(SizeType, "x"), FunctionArgument(SizeType, "y") },
		nullptr, "UISizeVec.new", &UISizeVec_new));

	To.addClassConstructor(SizeVecType, NativeFunction({ FunctionArgument(SizeType, "xy") },
		nullptr, "UISizeVec.new.same", &UISizeVec_new_same));
	To.addType(SizeVecType);

	auto UIBoxType = To.createClass<UIBox*>("UIBox");

	UIBoxType->members.push_back(ClassMember{
		.name = "isVisible",
		.offset = DS_OFFSETOF(UIBox, IsVisible),
		.type = BoolInst
		});

	UIBoxType->members.push_back(ClassMember{
		.name = "isCollapsed",
		.offset = DS_OFFSETOF(UIBox, IsCollapsed),
		.type = BoolInst
		});

	UIBoxType->members.push_back(ClassMember{
		.name = "isKeyboardFocusable",
		.offset = DS_OFFSETOF(UIBox, KeyboardFocusable),
		.type = BoolInst
		});

	UIBoxType->members.push_back(ClassMember{
		.name = "hasMouseCollision",
		.offset = DS_OFFSETOF(UIBox, HasMouseCollision),
		.type = BoolInst
		});

	To.addClassConstructor(UIBoxType, NativeFunction({ FunctionArgument(BoolInst, "horizontal") },
		nullptr, "UIBox.new", &UIBox_new));

	auto AddUIMethod = [&To](ClassType* type, NativeFunction fn) {
		fn.isDiscardable = true;

		To.addClassMethod(type, fn);
	};

	To.addClassMethod(UIBoxType, NativeFunction({ },
		SizeVecType, "getUsedSize", &UIBox_getUsedSize));

	AddUIMethod(UIBoxType, NativeFunction({ FunctionArgument(UIBoxType, "child") },
		UIBoxType, "addChild", &UIBox_addChild));

	AddUIMethod(UIBoxType, NativeFunction({ FunctionArgument(SizeVecType, "size") },
		UIBoxType, "setMinSize", &UIBox_setMinSize));

	AddUIMethod(UIBoxType, NativeFunction({ FunctionArgument(SizeVecType, "size") },
		UIBoxType, "setMaxSize", &UIBox_setMaxSize));

	AddUIMethod(UIBoxType, NativeFunction({
		FunctionArgument(SizeType, "left"), FunctionArgument(SizeType, "right"),
		FunctionArgument(SizeType, "up"), FunctionArgument(SizeType, "down") },
		UIBoxType, "setPadding", &UIBox_setPadding));

	AddUIMethod(UIBoxType, NativeFunction({ FunctionArgument(SizeType, "padding") },
		UIBoxType, "setAllPadding", &UIBox_setAllPadding));

	auto UITextType = To.createClass<UIText*>("UIText", UIBoxType);

	To.addClassConstructor(UITextType, NativeFunction(
		{ FunctionArgument(SizeType, "size"), FunctionArgument(Vec3Type, "color"), FunctionArgument(StrType, "text") },
		nullptr, "UIText.new", &UIText_new));

	AddUIMethod(UITextType, NativeFunction({ FunctionArgument(StrType, "newText") },
		UITextType, "setText", &UIText_setText));

	auto UIBackgroundType = To.createClass<UIBackground*>("UIBackground", UIBoxType);
	To.addClassConstructor(UIBackgroundType,
		NativeFunction({ FunctionArgument(BoolInst, "horizontal"), FunctionArgument(Vec3Type, "color") },
			nullptr, "UIBackground.new", &UIBackground_new));

	AddUIMethod(UIBackgroundType,
		NativeFunction({ FunctionArgument(Vec3Type, "newColor") },
			UITextType, "setColor", &UIBackground_setColor));

	AddUIMethod(UIBackgroundType,
		NativeFunction({ FunctionArgument(Vec3Type, "newColor") },
			UITextType, "setBorder", & UIBackground_setBorder));

	auto UIButtonType = To.createClass<UIBackground*>("UIButton", UIBoxType);
	To.addClassConstructor(UIButtonType,
		NativeFunction({ FunctionArgument(BoolInst, "horizontal"), FunctionArgument(Vec3Type, "color"),
			FunctionArgument(FunctionType::getInstance(nullptr, {}, ToContext->registry), "onClicked") },
			nullptr, "UIButton.new", &UIButton_new));

	AddUIMethod(UIButtonType,
		NativeFunction({ FunctionArgument(FunctionType::getInstance(nullptr, {}, ToContext->registry), "onClicked") },
		UITextType, "setOnClicked", &UIButton_setOnClicked));

	auto CanvasType = To.createClass<ScriptUICanvas*>("Canvas");

	CanvasType->baseConstructor = To.addFunction(NativeFunction({}, nullptr, "Canvas.new", &UIScriptCanvas_new));

	To.addClassMethod(CanvasType,
		NativeGenericFunction({ FunctionArgument(StrType, "name") }, { GenericArgument("T", UIBoxType) },
			GenericArgumentType::getInstance(0, true), "getChild", &UIScriptCanvas_getChild));

	To.addClassVirtualMethod(CanvasType,
		NativeFunction({}, nullptr, "update", &UIScriptCanvas_empty), 1);

	auto ElementType = To.createClass<ScriptUICanvas*>("UIScriptElement", UIBoxType);

	ElementType->baseConstructor = To.addFunction(NativeFunction({}, nullptr, "UIScriptElement.new", &UIScriptElement_new));

	To.addClassMethod(ElementType,
		NativeGenericFunction({ FunctionArgument(StrType, "name") }, { GenericArgument("T", UIBoxType) },
			GenericArgumentType::getInstance(0, true), "getChild", & UIScriptElement_getChild));

	To.addFunction(NativeFunction({ FunctionArgument(FloatInst, "pixelValue") }, SizeType, "pixels", UI_pixels));

	return UIBindings{
		.UITextType = UITextType,
		.UISizeType = SizeType,
	};
}
