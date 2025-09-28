#include "EngineModules.h"
#include <Engine/Objects/Components/MeshComponent.h>
#include <Engine/Objects/Components/MoveComponent.h>
#include <Engine/Objects/Components/CameraComponent.h>
#include <Engine/Objects/SceneObject.h>
#include <Engine/Input.h>
#include <Engine/Stats.h>
#include <language.hpp>
#include <modules/system.hpp>
#include <native/nativeModule.hpp>
#include <native/nativeStructType.hpp>
#include <parser/types/stringType.hpp>
#include <Core/Log.h>
#include <Engine/Script/ScriptObject.h>

using namespace lang;
using namespace engine::input;
using namespace engine;

class ExportAttribute : public modules::system::ReflectAttribute
{
public:
	ExportAttribute()
	{
		this->name = "Export";
	}
};

static void SceneObject_empty(InterpretContext* context)
{
	context->popValue<RuntimeClass*>();
}

static void SceneObject_attach(InterpretContext* context)
{
	ClassRef<script::ScriptObjectData*> Data = context->popValue<RuntimeClass*>();
	ClassRef<ObjectComponent*> Component = context->popValue<RuntimeClass*>();
	Data.getValue()->Parent->Attach(Component.getValue());
}

static void MeshComponent_new(InterpretContext* context)
{
	ClassRef<MeshComponent*> Component = context->popValue<RuntimeClass*>();
	Component.getValue() = new MeshComponent();
	context->pushValue(Component);
}

static void MeshComponent_load(InterpretContext* context)
{
	ClassRef<MeshComponent*> Component = context->popValue<RuntimeClass*>();
	ClassPtr<AssetRef*> File = context->popValue<RuntimeClass*>();

	Component.getValue()->Load(**File.get());
}

static void MoveComponent_new(InterpretContext* context)
{
	ClassRef<MoveComponent*> Component = context->popValue<RuntimeClass*>();
	Component.getValue() = new MoveComponent();
	context->pushValue(Component);
}

static void MoveComponent_addInput(InterpretContext* context)
{
	ClassRef<MoveComponent*> Component = context->popValue<RuntimeClass*>();
	Vector3 Direction = context->popValue<Vector3>();

	Component.getValue()->AddMovementInput(Direction);
}

static void MoveComponent_jump(InterpretContext* context)
{
	ClassRef<MoveComponent*> Component = context->popValue<RuntimeClass*>();

	Component.getValue()->Jump();
}

static void CameraComponent_new(InterpretContext* context)
{
	ClassRef<CameraComponent*> Component = context->popValue<RuntimeClass*>();
	Component.getValue() = new CameraComponent();
	context->pushValue(Component);
}

static void CameraComponent_use(InterpretContext* context)
{
	ClassRef<CameraComponent*> Component = context->popValue<RuntimeClass*>();
	Component.getValue()->Use();
}

static void CameraComponent_setFOV(InterpretContext* context)
{
	ClassRef<CameraComponent*> Component = context->popValue<RuntimeClass*>();
	float FOV = context->popValue<float>();
	Component.getValue()->SetFov(FOV);
}

static void Log_Info(InterpretContext* context)
{
	auto message = context->popRuntimeString();

	Log::Info(string(message.ptr(), message.length()));
}

static void Log_Warn(InterpretContext* context)
{
	auto message = context->popRuntimeString();

	Log::Warn(string(message.ptr(), message.length()));
}

static void Stats_GetDelta(InterpretContext* context)
{
	context->pushValue(stats::DeltaTime);
}

#pragma region Input

static void Input_IsKeyDown(InterpretContext* context)
{
	context->pushValue<Bool>(IsKeyDown(context->popValue<Key>()));
}

static void Input_GetMouseMovement(InterpretContext* context)
{
	context->pushValue<Vector2>(MouseMovement);
}

static void Input_IsLMBDown(InterpretContext* context)
{
	context->pushValue<Bool>(IsLMBDown);
}

static void Input_IsLMBClicked(InterpretContext* context)
{
	context->pushValue<Bool>(IsLMBClicked);
}

static void Input_IsRMBDown(InterpretContext* context)
{
	context->pushValue<Bool>(IsRMBDown);
}

static void Input_IsRMBClicked(InterpretContext* context)
{
	context->pushValue<Bool>(IsRMBClicked);
}

#pragma endregion

static void WriteVec(InterpretContext* context)
{
	Log::Info(context->popValue<Vector3>().ToString());
}

static void Vector3_Length(InterpretContext* context)
{
	context->pushValue(context->popValue<Vector3>().Length());
}

static void AssetRef_delete(InterpretContext* context)
{
	ClassPtr<AssetRef*> Ref = context->popValue<RuntimeClass*>();
	auto ref = *Ref.get();
	delete ref;
}

static VTableEntry AssetRef_vTable = VTableEntry{
	.nativeFn = &AssetRef_delete
};

static void AssetRef_new(InterpretContext* context)
{
	ClassRef<AssetRef*> NewAssetRef = context->popValue<RuntimeClass*>();
	NewAssetRef.classPtr->vtable = &AssetRef_vTable;
	RuntimeStr FilePath = context->popValue<RuntimeClass*>();

	NewAssetRef.getValue() = new AssetRef(AssetRef::FromPath(string(FilePath.ptr(), FilePath.length())));
	context->pushValue(NewAssetRef);
}

static void AssetRef_emptyAsset(InterpretContext* context)
{
	RuntimeStr Extension = context->popRuntimeString();

	ClassRef<AssetRef*> Asset = script::CreateAssetRef();
	Asset.getValue()->Extension = string(Extension.ptr(), Extension.length());
	Asset.classPtr->addRef();
	context->pushValue(Asset);
}

#pragma region Vector3

static void Vector3_Forward(InterpretContext* context)
{
	context->pushValue(Vector3::Forward(context->popValue<Rotation3>().EulerVector()));
}

static void Vector3_Right(InterpretContext* context)
{
	context->pushValue(Vector3::Right(context->popValue<Rotation3>().EulerVector()));
}

static void Vector3_add(InterpretContext* context)
{
	context->pushValue(context->popValue<Vector3>() + context->popValue<Vector3>());
}

static void Vector3_multiply(InterpretContext* context)
{
	context->pushValue(context->popValue<Vector3>() * context->popValue<Vector3>());
}

static void Vector3_subtract(InterpretContext* context)
{
	Vector3 b = context->popValue<Vector3>();
	Vector3 a = context->popValue<Vector3>();
	context->pushValue(a - b);
}

static void Vector3_divide(InterpretContext* context)
{
	Vector3 b = context->popValue<Vector3>();
	Vector3 a = context->popValue<Vector3>();
	context->pushValue(a / b);
}

static void Vector3_UnaryMinus(InterpretContext* context)
{
	context->pushValue(-context->popValue<Vector3>());
}

static void Vector3_length(InterpretContext* context)
{
	context->pushValue(context->popValue<Vector3>().Length());
}

#pragma endregion

void engine::script::RegisterEngineModules(lang::LanguageContext* ToContext)
{
	NativeModule EngineModule;
	EngineModule.name = "engine";

	auto StrType = StringType::getInstance();
	auto FloatInst = FloatType::getInstance();

	auto VecType = LANG_CREATE_STRUCT(Vector3);

	LANG_STRUCT_MEMBER_NAME(VecType, Vector3, X, x, FloatInst);
	LANG_STRUCT_MEMBER_NAME(VecType, Vector3, Y, y, FloatInst);
	LANG_STRUCT_MEMBER_NAME(VecType, Vector3, Z, z, FloatInst);

	VecType->operators.push_back({
		Operator::add,
		EngineModule.addFunction(NativeFunction({FunctionArgument(VecType, "a"), FunctionArgument(VecType, "b")},
			VecType, "Vector3.add", &Vector3_add))
		});

	VecType->operators.push_back({
		Operator::multiply,
		EngineModule.addFunction(NativeFunction({FunctionArgument(VecType, "a"), FunctionArgument(VecType, "b")},
			VecType, "Vector3.multiply", &Vector3_multiply))
		});

	VecType->operators.push_back({
		Operator::subtract,
		EngineModule.addFunction(NativeFunction({FunctionArgument(VecType, "a"), FunctionArgument(VecType, "b")},
			VecType, "Vector3.subtract", &Vector3_subtract))
		});

	VecType->operators.push_back({
		Operator::divide,
		EngineModule.addFunction(NativeFunction({FunctionArgument(VecType, "a"), FunctionArgument(VecType, "b")},
			VecType, "Vector3.divide", &Vector3_divide))
		});

	VecType->operators.push_back({
		Operator::unaryMinus,
		EngineModule.addFunction(NativeFunction({FunctionArgument(VecType, "a")},
			VecType, "Vector3.unaryMinus", &Vector3_UnaryMinus))
		});

	VecType->methods.insert({ "length", EngineModule.addFunction(NativeFunction({},
			FloatInst, "Vector3.length", &Vector3_length)) });

	auto Vec2Type = LANG_CREATE_STRUCT(Vector2);

	LANG_STRUCT_MEMBER_NAME(Vec2Type, Vector2, X, x, FloatInst);
	LANG_STRUCT_MEMBER_NAME(Vec2Type, Vector2, Y, y, FloatInst);

	auto RotType = LANG_CREATE_STRUCT(Rotation3);

	LANG_STRUCT_MEMBER_NAME(RotType, Rotation3, P, p, FloatInst);
	LANG_STRUCT_MEMBER_NAME(RotType, Rotation3, Y, y, FloatInst);
	LANG_STRUCT_MEMBER_NAME(RotType, Rotation3, R, r, FloatInst);

	EngineModule.types.push_back(RotType);
	EngineModule.types.push_back(VecType);
	EngineModule.types.push_back(Vec2Type);

	auto AssetRefType = EngineModule.createClass<AssetRef*>("AssetRef");

	EngineModule.addClassConstructor(AssetRefType, NativeFunction{
		{FunctionArgument(StrType, "path")}, nullptr, "AssetRef.new",
		&AssetRef_new
		});

	auto ObjectType = EngineModule.createClass<ScriptObjectData>("SceneObject");
	auto ComponentType = EngineModule.createClass<ObjectComponent*>("ObjectComponent");

	EngineModule.addClassVirtualMethod(ObjectType,
		NativeFunction({}, nullptr, "begin", &SceneObject_empty), 1);

	EngineModule.addClassVirtualMethod(ObjectType,
		NativeFunction({}, nullptr, "destroy", &SceneObject_empty), 2);

	EngineModule.addClassVirtualMethod(ObjectType,
		NativeFunction({}, nullptr, "update", &SceneObject_empty), 3);

	ObjectType->members.push_back(ClassMember{
		.name = "position",
		.offset = offsetof(ScriptObjectData, Position),
		.type = VecType
		});

	ObjectType->members.push_back(ClassMember{
		.name = "rotation",
		.offset = offsetof(ScriptObjectData, Rotation),
		.type = RotType
		});

	EngineModule.addClassMethod(ObjectType,
		NativeFunction({ FunctionArgument(ComponentType, "component") }, nullptr,
			"attach", &SceneObject_attach));

	EngineModule.addFunction(
		NativeFunction({ FunctionArgument(StringType::getInstance(), "message") },
			nullptr, "info", &Log_Info));

	EngineModule.addFunction(
		NativeFunction({ FunctionArgument(StringType::getInstance(), "message") },
			nullptr, "warn", &Log_Warn));

	EngineModule.addFunction(
		NativeFunction({ },
			FloatInst, "getDelta", &Stats_GetDelta));

	auto MeshComponentType = EngineModule.createClass<MeshComponent*>("MeshComponent", ComponentType);

	EngineModule.addClassConstructor(MeshComponentType,
		NativeFunction({ },
			nullptr, "MeshComponent.new", &MeshComponent_new));

	EngineModule.addClassMethod(MeshComponentType,
		NativeFunction({ FunctionArgument(AssetRefType, "file") },
			nullptr, "load", &MeshComponent_load));

	auto MoveComponentType = EngineModule.createClass<MoveComponent*>("MoveComponent", ComponentType);

	EngineModule.addClassConstructor(MoveComponentType,
		NativeFunction({ },
			nullptr, "MoveComponent.new", &MoveComponent_new));

	EngineModule.addClassMethod(MoveComponentType,
		NativeFunction({ FunctionArgument(VecType, "direction") },
			nullptr, "addInput", &MoveComponent_addInput));

	EngineModule.addClassMethod(MoveComponentType,
		NativeFunction({ },
			nullptr, "jump", &MoveComponent_jump));

	auto CameraComponentType = EngineModule.createClass<MoveComponent*>("CameraComponent", ComponentType);

	EngineModule.addClassConstructor(CameraComponentType,
		NativeFunction({ },
			nullptr, "CameraComponent.new", &CameraComponent_new));

	EngineModule.addClassMethod(CameraComponentType,
		NativeFunction({ },
			nullptr, "use", &CameraComponent_use));

	EngineModule.addClassMethod(CameraComponentType,
		NativeFunction({ FunctionArgument(FloatInst, "fov") },
			nullptr, "setFOV", &CameraComponent_setFOV));

	EngineModule.addFunction(
		NativeFunction({ FunctionArgument(VecType, "message") },
			nullptr, "writeVec", &WriteVec));

	EngineModule.addFunction(
		NativeFunction({ FunctionArgument(FloatInst, "x"),FunctionArgument(FloatInst, "y"),FunctionArgument(FloatInst, "z") },
			VecType, "vec3", [](InterpretContext* context) {}));

	EngineModule.addFunction(
		NativeFunction({ FunctionArgument(RotType, "rotation") },
			VecType, "forward", &Vector3_Forward));

	EngineModule.addFunction(
		NativeFunction({ FunctionArgument(RotType, "rotation") },
			VecType, "right", &Vector3_Right));

	EngineModule.addFunction(
		NativeFunction({ FunctionArgument(FloatInst, "p"),FunctionArgument(FloatInst, "y"),FunctionArgument(FloatInst, "r") },
			RotType, "rot3", [](InterpretContext* context) {}));

	EngineModule.addFunction(
		NativeFunction({ FunctionArgument(StrType, "extension") },
			AssetRefType, "emptyAsset", AssetRef_emptyAsset));

	EngineModule.attributes.push_back(new ExportAttribute());

	NativeModule EngineInputModule;
	EngineInputModule.name = "engine::input";
	auto KeyType = EngineInputModule.createEnum("Key");
	EngineInputModule.addEnumValue(KeyType, "a", Key::a);
	EngineInputModule.addEnumValue(KeyType, "b", Key::b);
	EngineInputModule.addEnumValue(KeyType, "c", Key::c);
	EngineInputModule.addEnumValue(KeyType, "d", Key::d);
	EngineInputModule.addEnumValue(KeyType, "e", Key::e);
	EngineInputModule.addEnumValue(KeyType, "f", Key::f);
	EngineInputModule.addEnumValue(KeyType, "g", Key::g);
	EngineInputModule.addEnumValue(KeyType, "h", Key::h);
	EngineInputModule.addEnumValue(KeyType, "i", Key::i);
	EngineInputModule.addEnumValue(KeyType, "j", Key::j);
	EngineInputModule.addEnumValue(KeyType, "k", Key::k);
	EngineInputModule.addEnumValue(KeyType, "l", Key::l);
	EngineInputModule.addEnumValue(KeyType, "m", Key::m);
	EngineInputModule.addEnumValue(KeyType, "n", Key::n);
	EngineInputModule.addEnumValue(KeyType, "o", Key::p);
	EngineInputModule.addEnumValue(KeyType, "p", Key::p);
	EngineInputModule.addEnumValue(KeyType, "q", Key::q);
	EngineInputModule.addEnumValue(KeyType, "r", Key::r);
	EngineInputModule.addEnumValue(KeyType, "s", Key::s);
	EngineInputModule.addEnumValue(KeyType, "t", Key::t);
	EngineInputModule.addEnumValue(KeyType, "u", Key::u);
	EngineInputModule.addEnumValue(KeyType, "v", Key::v);
	EngineInputModule.addEnumValue(KeyType, "w", Key::w);
	EngineInputModule.addEnumValue(KeyType, "x", Key::x);
	EngineInputModule.addEnumValue(KeyType, "y", Key::y);
	EngineInputModule.addEnumValue(KeyType, "z", Key::z);

	EngineInputModule.addEnumValue(KeyType, "up", Key::UP);
	EngineInputModule.addEnumValue(KeyType, "down", Key::DOWN);
	EngineInputModule.addEnumValue(KeyType, "left", Key::LEFT);
	EngineInputModule.addEnumValue(KeyType, "right", Key::RIGHT);
	EngineInputModule.addFunction(NativeFunction({ FunctionArgument(KeyType, "toCheck") },
		BoolType::getInstance(), "isKeyDown", &Input_IsKeyDown));

	EngineInputModule.addFunction(NativeFunction({},
		BoolType::getInstance(), "isLMBDown", & Input_IsLMBDown));

	EngineInputModule.addFunction(NativeFunction({},
		BoolType::getInstance(), "isRMBDown", & Input_IsRMBDown));

	EngineInputModule.addFunction(NativeFunction({},
		BoolType::getInstance(), "isLMBClicked", & Input_IsLMBClicked));

	EngineInputModule.addFunction(NativeFunction({},
		BoolType::getInstance(), "isRMBClicked", & Input_IsRMBClicked));

	EngineInputModule.addFunction(NativeFunction({ },
		Vec2Type, "getMouseMovement", &Input_GetMouseMovement));

	ToContext->addNativeModule(EngineModule);
	ToContext->addNativeModule(EngineInputModule);
}

lang::RuntimeClass* engine::script::CreateAssetRef()
{
	ClassRef<AssetRef*> NewAssetRef = RuntimeClass::allocateClass(sizeof(AssetRef*), &AssetRef_vTable);
	NewAssetRef.classPtr->vtable = &AssetRef_vTable;

	NewAssetRef.getValue() = new AssetRef();
	return NewAssetRef.classPtr;
}
