#include "ScriptUICanvas.h"
#include "ScriptUIElement.h"
#include "UIBindings.h"
#include <ds/callableWrapper.hpp>
#include <ds/language.hpp>
#include <ds/native/nativeGeneric.hpp>
#include <ds/parser/types/functionType.hpp>
#include <Engine/Script/ScriptSubsystem.h>
#include <kui/KlemmUI.h>
#include <Core/Error/EngineError.h>
#include <kui/UI/UIStackBox.h>
#include <Engine/UI/Popup.h>

using namespace engine;
using namespace engine::script::ui;
using namespace engine::script;
using namespace ds;
using namespace kui;

static void UIBox_delete(InterpretContext* context)
{
	auto ptr = context->popPtr<UIBox*>();
	if (*ptr)
	{
		(*ptr)->RemoveDeletedListener(ptr.classPtr);
		ScriptSubsystem::Instance->UIObjectMappings.erase(*ptr);
	}
}

static RuntimeFunction UIBox_vTable = {
	.nativeFn = UIBox_delete,
};

ds::RuntimeClass* GetClassFromUIBox(UIBox* Object)
{
	if (!Object)
	{
		return NativeModule::makePointerClass<UIBox>(nullptr);
	}

	auto found = ScriptSubsystem::Instance->UIObjectMappings.find(Object);

	if (found != ScriptSubsystem::Instance->UIObjectMappings.end())
	{
		found->second->addRef();
		return found->second;
	}

	ds::ClassRef<UIBox*> NewObject = ds::RuntimeClass::allocateClass(sizeof(UIBox*), 0, &UIBox_vTable);
	NewObject.getValue() = Object;
	ScriptSubsystem::Instance->UIObjectMappings[Object] = NewObject.classPtr;
	Object->AddDeletedListener(NewObject.classPtr, [Object, NewObject] {
		ScriptSubsystem::Instance->UIObjectMappings.erase(Object);
		NewObject.getValue() = nullptr;
	});
	return NewObject.classPtr;
}

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

static void UIScriptCanvas_getRoot(InterpretContext* context)
{
	ClassRef<ScriptUICanvas*> cls = context->popValue<RuntimeClass*>();
	context->pushValue(GetClassFromUIBox(cls.getValue()->MarkupBox));
}

static void UIScriptCanvas_sizeToPosition(InterpretContext* context)
{
	ClassRef<ScriptUICanvas*> cls = context->popValue<RuntimeClass*>();
	auto SizeValue = context->popValue<SizeVec>();

	auto Size = cls.getValue()->CanvasBox->GetUsedSize().GetScreen();

	Vec2f Converted = SizeValue.GetScreen() / Size * 2;

	context->pushValue<Vec2f>(Converted);
}

static void UIScriptCanvas_destroy(InterpretContext* context)
{
	ClassRef<ScriptUICanvas*> cls = context->popValue<RuntimeClass*>();
	delete cls.getValue();
	cls.getValue() = nullptr;
}

static void UIScriptCanvas_getChild(InterpretContext* context)
{
	auto type = GenericData(context);
	ClassRef<ScriptUICanvas*> cls = context->popValue<RuntimeClass*>();
	auto text = context->popRuntimeString();

	if (!cls.getValue())
	{
		return;
	}

	auto classInstance = cls.getValue();
	auto box = classInstance->MarkupBox;

	auto found = box->NamedChildren[text.ptr()];

	if (!found)
	{
		context->runtimePanic("UIScriptCanvas.getChild() failed.");
		return;
	}

	context->pushValue(GetClassFromUIBox(found));
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

	auto found = classInstance->NamedChildren[text.ptr()];

	if (ScriptSubsystem::Instance->UIObjectMappings.contains(found))
	{
		context->pushValue(ScriptSubsystem::Instance->UIObjectMappings[found]);
		return;
	}

	if (!found)
	{
		context->runtimePanic("UIScriptElement.getChild() failed.");
		return;
	}

	context->pushValue(GetClassFromUIBox(found));
}

static void UIScriptElement_setString(InterpretContext* context)
{
	ClassRef<ScriptUIElement*> cls = context->popValue<RuntimeClass*>();

	auto text = context->popRuntimeString();
	auto name = context->popRuntimeString();

	cls.getValue()->SetVariable(name.ptr(), AnyContainer(text.ptr()));
}

static void UIScriptElement_setVec3(InterpretContext* context)
{
	ClassRef<ScriptUIElement*> cls = context->popValue<RuntimeClass*>();

	kui::Vec3f vec3 = context->popValue<kui::Vec3f>();
	auto name = context->popRuntimeString();

	cls.getValue()->SetVariable(name.ptr(), AnyContainer(vec3));
}

static void UIScriptElement_setSizeVec(InterpretContext* context)
{
	ClassRef<ScriptUIElement*> cls = context->popValue<RuntimeClass*>();

	SizeVec size = context->popValue<SizeVec>();
	auto name = context->popRuntimeString();

	cls.getValue()->SetVariable(name.ptr(), AnyContainer(size));
}

static void UIScriptElement_setSize(InterpretContext* context)
{
	ClassRef<ScriptUIElement*> cls = context->popValue<RuntimeClass*>();

	UISize size = context->popValue<UISize>();
	auto name = context->popRuntimeString();

	cls.getValue()->SetVariable(name.ptr(), AnyContainer(size));
}

static void UIBox_new(InterpretContext* context)
{
	ClassRef<UIBox*> cls = context->popValue<RuntimeClass*>();

	Bool horizontal = context->popValue<Bool>();

	cls.getValue() = new UIBox(horizontal);

	context->pushValue(cls);
}

static void UICanvasBox_new(InterpretContext* context)
{
	ClassRef<UICanvasBox*> cls = context->popValue<RuntimeClass*>();

	SizeVec size = context->popValue<SizeVec>();

	cls.getValue() = new UICanvasBox(0, size);

	context->pushValue(cls);
}

static void UIBox_deleteChildren(InterpretContext* context)
{
	ds::ClassRef<UIBox*> cls = context->popValue<RuntimeClass*>();
	cls.getValue()->DeleteChildren();
}

static void UIBox_destroy(InterpretContext* context)
{
	ds::ClassRef<UIBox*> cls = context->popValue<RuntimeClass*>();
	delete cls.getValue();
	cls.getValue() = nullptr;
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

static void UIBox_setPosition(InterpretContext* context)
{
	ClassRef<UIBox*> cls = context->popValue<RuntimeClass*>();
	Vec2f size = context->popValue<Vec2f>();

	cls.getValue()->SetPosition(size);

	cls.classPtr->addRef();
	context->pushValue(cls);
}

static void UIBox_setMinWidth(InterpretContext* context)
{
	ClassRef<UIBox*> cls = context->popValue<RuntimeClass*>();
	UISize size = context->popValue<UISize>();

	cls.getValue()->SetMinWidth(size);

	cls.classPtr->addRef();
	context->pushValue(cls);
}

static void UIBox_setMinHeight(InterpretContext* context)
{
	ClassRef<UIBox*> cls = context->popValue<RuntimeClass*>();
	UISize size = context->popValue<UISize>();

	cls.getValue()->SetMinHeight(size);

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

static void UIBox_updateLayout(InterpretContext* context)
{
	ClassRef<UIBox*> cls = context->popValue<RuntimeClass*>();

	cls.getValue()->GetAbsoluteParent()->UpdateElement();
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

static void UIStackBox_new(InterpretContext* context)
{
	ClassRef<UIStackBox*> cls = context->popValue<RuntimeClass*>();
	UISize Size = context->popValue<UISize>();
	Vec2f Position = context->popValue<Vec2f>();

	cls.getValue() = new UIStackBox(Position, Size);

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

static void UIBlurBackground_new(InterpretContext* context)
{
	ClassRef<UIBlurBackground*> cls = context->popValue<RuntimeClass*>();
	Vec3f Color = context->popValue<Vec3f>();
	Bool Horizontal = context->popValue<Bool>();

	cls.getValue() = new UIBlurBackground(Horizontal, 0, Color);

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

static void UIScrollBox_new(InterpretContext* context)
{
	ClassRef<UIScrollBox*> cls = context->popValue<RuntimeClass*>();
	Bool ShowScrollBar = context->popValue<Bool>();
	Bool Horizontal = context->popValue<Bool>();

	cls.getValue() = new UIScrollBox(Horizontal, 0, ShowScrollBar);

	context->pushValue(cls);
}

static void UITextField_new(InterpretContext* context)
{
	ClassRef<UITextField*> cls = context->popValue<RuntimeClass*>();
	Vec3f Color = context->popValue<Vec3f>();

	cls.getValue() = new UITextField(0, Color, VideoSubsystem::Current->DefaultFont, nullptr);

	context->pushValue(cls);
}

static void UITextField_setOnSubmit(InterpretContext* context)
{
	ClassRef<UITextField*> cls = context->popValue<RuntimeClass*>();
	CallableWrapper<void> OnSubmit = { context->popValue<RuntimeClass*>(), context };

	cls.getValue()->OnChanged = OnSubmit;

	cls.classPtr->addRef();
	context->pushValue(cls);
}

static void UITextField_setOnChanged(InterpretContext* context)
{
	ClassRef<UITextField*> cls = context->popValue<RuntimeClass*>();
	CallableWrapper<void> OnChanged = { context->popValue<RuntimeClass*>(), context };

	cls.getValue()->OnValueChanged = OnChanged;

	cls.classPtr->addRef();
	context->pushValue(cls);
}

static void UITextField_getText(InterpretContext* context)
{
	ClassRef<UITextField*> cls = context->popValue<RuntimeClass*>();

	context->pushRuntimeString(RuntimeStr(cls.getValue()->GetText().c_str()));
}

static void UITextField_setText(InterpretContext* context)
{
	ClassRef<UITextField*> cls = context->popValue<RuntimeClass*>();
	RuntimeStr text = context->popRuntimeString();
	cls.getValue()->SetText(text.ptr());
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

static void UISize_getScreen(InterpretContext* context)
{
	UISize vec = context->popValue<UISize>();

	context->pushValue(vec.GetScreen());
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

static void UISizeVec_new_screen(InterpretContext* context)
{
	Vec2f xy = context->popValue<Vec2f>();

	context->pushValue(SizeVec(xy));
}
static void UISizeVec_getScreen(InterpretContext* context)
{
	SizeVec vec = context->popValue<SizeVec>();

	context->pushValue(vec.GetScreen());
}

static void UISize_pixels(InterpretContext* context)
{
	context->pushValue(context->popValue<UISize>().GetPixels());
}

static void UI_pixels(InterpretContext* context)
{
	context->pushValue(UISize::Pixels(context->popValue<Float>()));
}

static void UI_makePopup(InterpretContext* context)
{
	ClassPtr<UIBox*> bx = context->popPtr<UIBox*>();

	auto NewPopup = new Popup();
	NewPopup->SetBox(*bx);
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
	To.addClassMethod(SizeType, NativeFunction({},
		Vec2Type, "getScreen", &UISize_getScreen));
	To.addType(SizeType);

	auto SizeVecType = new NativeStructType(sizeof(SizeVec), "UISizeVec");
	DS_STRUCT_MEMBER_NAME(SizeVecType, SizeVec, X, x, SizeType);
	DS_STRUCT_MEMBER_NAME(SizeVecType, SizeVec, Y, y, SizeType);

	To.addClassConstructor(SizeVecType, NativeFunction({ FunctionArgument(SizeType, "x"), FunctionArgument(SizeType, "y") },
		nullptr, "UISizeVec.new", &UISizeVec_new));

	To.addClassConstructor(SizeVecType, NativeFunction({ FunctionArgument(SizeType, "xy") },
		nullptr, "UISizeVec.new.same", &UISizeVec_new_same));
	To.addClassConstructor(SizeVecType, NativeFunction({ FunctionArgument(SizeType, "xy") },
		nullptr, "UISizeVec.new.screen", &UISizeVec_new_screen));
	To.addClassMethod(SizeVecType, NativeFunction({},
		Vec2Type, "getScreen", &UISizeVec_getScreen));
	To.addType(SizeVecType);

	// UIBox
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

	UIBoxType->makePointerClass();

	auto AddUIMethod = [&To](ClassType* type, NativeFunction fn) {
		fn.isDiscardable = true;
		fn.returnType = type;

		To.addClassMethod(type, fn);
	};

	To.addClassMethod(UIBoxType, NativeFunction({},
		SizeVecType, "getUsedSize", &UIBox_getUsedSize));

	AddUIMethod(UIBoxType, NativeFunction({ FunctionArgument(UIBoxType, "child") },
		UIBoxType, "addChild", &UIBox_addChild));

	To.addClassMethod(UIBoxType, NativeFunction({},
		nullptr, "deleteChildren", &UIBox_deleteChildren));

	To.addClassMethod(UIBoxType, NativeFunction({},
		nullptr, "destroy", &UIBox_destroy));

	AddUIMethod(UIBoxType, NativeFunction({ FunctionArgument(Vec2Type, "size") },
		UIBoxType, "setPosition", &UIBox_setPosition));

	AddUIMethod(UIBoxType, NativeFunction({ FunctionArgument(SizeVecType, "size") },
		UIBoxType, "setMinSize", &UIBox_setMinSize));

	AddUIMethod(UIBoxType, NativeFunction({ FunctionArgument(SizeType, "size") },
		UIBoxType, "setMinWidth", &UIBox_setMinWidth));

	AddUIMethod(UIBoxType, NativeFunction({ FunctionArgument(SizeType, "size") },
		UIBoxType, "setMinHeight", &UIBox_setMinHeight));

	AddUIMethod(UIBoxType, NativeFunction({ FunctionArgument(SizeVecType, "size") },
		UIBoxType, "setMaxSize", &UIBox_setMaxSize));

	To.addClassMethod(UIBoxType, NativeFunction({},
		nullptr, "updateLayout", &UIBox_updateLayout));

	AddUIMethod(UIBoxType, NativeFunction({
		FunctionArgument(SizeType, "left"), FunctionArgument(SizeType, "right"),
		FunctionArgument(SizeType, "up"), FunctionArgument(SizeType, "down") },
		UIBoxType, "setPadding", &UIBox_setPadding));

	AddUIMethod(UIBoxType, NativeFunction({ FunctionArgument(SizeType, "padding") },
		UIBoxType, "setAllPadding", &UIBox_setAllPadding));

	// UIText
	auto UITextType = To.createClass<UIText*>("UIText", UIBoxType);

	To.addClassConstructor(UITextType, NativeFunction(
		{ FunctionArgument(SizeType, "size"), FunctionArgument(Vec3Type, "color"), FunctionArgument(StrType, "text") },
		nullptr, "UIText.new", &UIText_new));

	AddUIMethod(UITextType, NativeFunction({ FunctionArgument(StrType, "newText") },
		UITextType, "setText", &UIText_setText));

	AddUIMethod(UITextType, NativeFunction({ FunctionArgument(Vec3Type, "newColor") },
		UITextType, "setColor", &UIText_setColor));

	// UIBackground
	auto UIBackgroundType = To.createClass<UIBackground*>("UIBackground", UIBoxType);
	To.addClassConstructor(UIBackgroundType,
		NativeFunction({ FunctionArgument(BoolInst, "horizontal"), FunctionArgument(Vec3Type, "color") },
			nullptr, "UIBackground.new", &UIBackground_new));

	AddUIMethod(UIBackgroundType,
		NativeFunction({ FunctionArgument(Vec3Type, "newColor") },
			UIBackgroundType, "setColor", &UIBackground_setColor));

	AddUIMethod(UIBackgroundType,
		NativeFunction({ FunctionArgument(SizeType, "newSize"), FunctionArgument(Vec3Type, "newColor") },
			UIBackgroundType, "setBorder", &UIBackground_setBorder));

	// UIBlurBackground
	auto UIBlurBackgroundType = To.createClass<UIBlurBackground*>("UIBlurBackground", UIBackgroundType);
	To.addClassConstructor(UIBlurBackgroundType,
		NativeFunction({ FunctionArgument(BoolInst, "horizontal"), FunctionArgument(Vec3Type, "color") },
			nullptr, "UIBlurBackground.new", &UIBlurBackground_new));

	// UIScrollBox
	auto UIScrollBoxType = To.createClass<UIScrollBox*>("UIScrollBox", UIBoxType);

	To.addClassConstructor(UIScrollBoxType, NativeFunction(
		{ FunctionArgument(BoolInst, "horizontal"), FunctionArgument(BoolInst, "showScrollBar") },
		nullptr, "UIScrollBox.new", &UIScrollBox_new));

	// UICanvasBox
	auto UICanvasBoxType = To.createClass<UICanvasBox*>("UICanvasBox", UIBoxType);

	To.addClassConstructor(UICanvasBoxType, NativeFunction(
		{ FunctionArgument(SizeType, "size") },
		nullptr, "UICanvasBox.new", &UICanvasBox_new));

	// UIStackBox
	auto UIStackBoxType = To.createClass<UICanvasBox*>("UIStackBox", UIBoxType);

	To.addClassConstructor(UIStackBoxType, NativeFunction(
		{ FunctionArgument(Vec2Type, "position"), FunctionArgument(SizeType, "size") },
		nullptr, "UIStackBox.new", &UIStackBox_new));

	// UITextField
	auto UITextFieldType = To.createClass<UIText*>("UITextField", UIBackgroundType);
	To.addClassConstructor(UITextFieldType, NativeFunction(
		{ FunctionArgument(Vec3Type, "color") },
		nullptr, "UITextField.new", &UITextField_new));

	AddUIMethod(UITextFieldType, NativeFunction(
		{ FunctionArgument(FunctionType::getInstance(nullptr,{}, ToContext->registry), "callback") },
		UITextFieldType, "setOnSubmit", &UITextField_setOnSubmit));

	AddUIMethod(UITextFieldType, NativeFunction(
		{ FunctionArgument(FunctionType::getInstance(nullptr,{}, ToContext->registry), "callback") },
		UITextFieldType, "setOnChanged", &UITextField_setOnChanged));

	AddUIMethod(UITextFieldType, NativeFunction(
		{ FunctionArgument(StrType, "newText") },
		UITextFieldType, "setText", &UITextField_setText));

	To.addClassMethod(UITextFieldType,
		NativeFunction({},
			StrType, "getText", &UITextField_getText));

	// UIButton
	auto UIButtonType = To.createClass<UIBackground*>("UIButton", UIBackgroundType);
	To.addClassConstructor(UIButtonType,
		NativeFunction({ FunctionArgument(BoolInst, "horizontal"), FunctionArgument(Vec3Type, "color"),
			FunctionArgument(FunctionType::getInstance(nullptr,{}, ToContext->registry), "onClicked") },
			nullptr, "UIButton.new", &UIButton_new));

	AddUIMethod(UIButtonType,
		NativeFunction({ FunctionArgument(FunctionType::getInstance(nullptr,{}, ToContext->registry), "onClicked") },
			UIButtonType, "setOnClicked", &UIButton_setOnClicked));

	// Script canvas
	auto CanvasType = To.createClass<ScriptUICanvas*>("Canvas");

	CanvasType->baseConstructor = To.addFunction(NativeFunction({}, nullptr, "Canvas.new", &UIScriptCanvas_new));

	To.addClassMethod(CanvasType,
		NativeGenericFunction({ FunctionArgument(StrType, "name") }, { GenericArgument("T", UIBoxType) },
			GenericArgumentType::getInstance(0, true), ".getChild", &UIScriptCanvas_getChild));

	To.addClassMethod(CanvasType,
		NativeFunction({},
			nullptr, "destroy", &UIScriptCanvas_destroy));

	To.addClassVirtualMethod(CanvasType,
		NativeFunction({}, nullptr, "update", &UIScriptCanvas_empty), 1);

	To.addClassMethod(CanvasType,
		NativeFunction({},
			UIBoxType, "getRoot", &UIScriptCanvas_getRoot));

	To.addClassMethod(CanvasType,
		NativeFunction({ FunctionArgument(SizeVecType, "vec") },
			Vec2Type, "sizeToPosition", &UIScriptCanvas_sizeToPosition));

	auto ElementType = To.createClass<ScriptUICanvas*>("UIScriptElement", UIBoxType);

	ElementType->baseConstructor = To.addFunction(NativeFunction({}, nullptr, "UIScriptElement.new", &UIScriptElement_new));

	To.addClassMethod(ElementType,
		NativeGenericFunction({ FunctionArgument(StrType, "name") }, { GenericArgument("T", UIBoxType) },
			GenericArgumentType::getInstance(0, true), ".getChild", &UIScriptElement_getChild));

	To.addClassMethod(ElementType,
		NativeFunction({ FunctionArgument(StrType, "name"), FunctionArgument(StrType, "value") },
			nullptr, "setString", &UIScriptElement_setString));

	To.addClassMethod(ElementType,
		NativeFunction({ FunctionArgument(StrType, "name"), FunctionArgument(Vec3Type, "value") },
			nullptr, "setVec3", &UIScriptElement_setVec3));

	To.addClassMethod(ElementType,
		NativeFunction({ FunctionArgument(StrType, "name"), FunctionArgument(SizeVecType, "value") },
			nullptr, "setSizeVec", &UIScriptElement_setSizeVec));

	To.addClassMethod(ElementType,
		NativeFunction({ FunctionArgument(StrType, "name"), FunctionArgument(SizeType, "value") },
			nullptr, "setSize", &UIScriptElement_setSizeVec));

	To.addFunction(NativeFunction({ FunctionArgument(FloatInst, "pixelValue") }, SizeType, "pixels", UI_pixels));

	To.addFunction(NativeFunction({ FunctionArgument(UIBoxType, "box") }, nullptr, "makePopup", UI_makePopup));

	return UIBindings{
		.UITextType = UITextType,
		.UISizeType = SizeType,
	};
}
