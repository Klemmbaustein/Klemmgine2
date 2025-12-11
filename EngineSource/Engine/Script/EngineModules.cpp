#include "EngineModules.h"
#include <Engine/Objects/Components/MeshComponent.h>
#include <Engine/Objects/Components/MoveComponent.h>
#include <Engine/Objects/Components/CameraComponent.h>
#include <Engine/Objects/SceneObject.h>
#include <Engine/Script/ScriptSubsystem.h>
#include <Engine/Input.h>
#include <Engine/Engine.h>
#include <Engine/Stats.h>
#include <ds/language.hpp>
#include <ds/modules/system.hpp>
#include <ds/modules/system.async.hpp>
#include <ds/native/nativeModule.hpp>
#include <ds/native/nativeStructType.hpp>
#include <ds/parser/types/stringType.hpp>
#include <ds/parser/types/taskType.hpp>
#include <Core/Log.h>
#include <Engine/Script/ScriptObject.h>

using namespace ds;
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
	ClassPtr<ObjectComponent*> Component = context->popPtr<ObjectComponent*>();
	Data.getValue()->Parent->Attach(*Component);
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
	ClassPtr<AssetRef*> File = context->popPtr<AssetRef*>();

	Component.getValue()->Load(**File);
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
	ClassPtr<AssetRef*> Ref = context->popPtr<AssetRef*>();
	auto ref = *Ref.get();
	delete ref;
}

static void Wait(InterpretContext* context)
{
	using namespace ds::modules::system::async;

	Float time = context->popValue<Float>();

	if (time <= 0)
	{
		context->pushValue(completedTask());
		return;
	}

	auto t = emptyTask();

	t->addRef();

	Engine::GetSubsystem<script::ScriptSubsystem>()->WaitTasks.push_back(script::WaitTask{
		.TaskObject = t,
		.Time = time,
		});

	context->pushValue(t);
}

static RuntimeFunction AssetRef_vTable = RuntimeFunction{
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

void engine::script::RegisterEngineModules(ds::LanguageContext* ToContext)
{
	NativeModule EngineModule;
	EngineModule.name = "engine";

	auto StrType = StringType::getInstance();
	auto FloatInst = FloatType::getInstance();

	auto VecType = DS_CREATE_STRUCT(Vector3);

	DS_STRUCT_MEMBER_NAME(VecType, Vector3, X, x, FloatInst);
	DS_STRUCT_MEMBER_NAME(VecType, Vector3, Y, y, FloatInst);
	DS_STRUCT_MEMBER_NAME(VecType, Vector3, Z, z, FloatInst);

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

	EngineModule.addStructMethod(VecType, NativeFunction({},
		FloatInst, "length", &Vector3_length));

	auto Vec3Function = EngineModule.addFunction(
		NativeFunction({ FunctionArgument(FloatInst, "x"),FunctionArgument(FloatInst, "y"),FunctionArgument(FloatInst, "z") },
			VecType, "vec3", [](InterpretContext* context) {}));

	VecType->addConstructor(Vec3Function);
	VecType->addConstructor(EngineModule.addFunction(
		NativeFunction({ FunctionArgument(FloatInst, "xyz") },
			VecType, "Vector3.new.xyz", [](InterpretContext* context) {
		float xyz = context->popValue<Float>();

		context->pushValue(Vector3(xyz));
	})));

	auto Vec2Type = DS_CREATE_STRUCT(Vector2);

	DS_STRUCT_MEMBER_NAME(Vec2Type, Vector2, X, x, FloatInst);
	DS_STRUCT_MEMBER_NAME(Vec2Type, Vector2, Y, y, FloatInst);


	auto Vec2Function = EngineModule.addFunction(
		NativeFunction({ FunctionArgument(FloatInst, "x"),FunctionArgument(FloatInst, "y") },
			Vec2Type, "vec2", [](InterpretContext* context) {}));

	Vec2Type->addConstructor(Vec2Function);

	auto RotType = DS_CREATE_STRUCT(Rotation3);

	DS_STRUCT_MEMBER_NAME(RotType, Rotation3, P, p, FloatInst);
	DS_STRUCT_MEMBER_NAME(RotType, Rotation3, Y, y, FloatInst);
	DS_STRUCT_MEMBER_NAME(RotType, Rotation3, R, r, FloatInst);

	auto Rot3Function = EngineModule.addFunction(
		NativeFunction({ FunctionArgument(FloatInst, "p"),FunctionArgument(FloatInst, "y"),FunctionArgument(FloatInst, "r") },
			RotType, "rot3", [](InterpretContext* context) {}));

	RotType->addConstructor(Rot3Function);

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

	ComponentType->members.push_back(ClassMember{
		.name = "position",
		.offset = offsetof(ObjectComponent, Position),
		.type = VecType
		});

	ComponentType->members.push_back(ClassMember{
		.name = "rotation",
		.offset = offsetof(ObjectComponent, Rotation),
		.type = RotType
		});

	ComponentType->members.push_back(ClassMember{
		.name = "scale",
		.offset = offsetof(ObjectComponent, Scale),
		.type = VecType
		});

	ComponentType->isPointerClass = true;

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
		NativeFunction({ FunctionArgument(FloatInst, "timeInSeconds")},
			TaskType::getInstance(nullptr), "wait", &Wait));

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
		NativeFunction({ FunctionArgument(RotType, "rotation") },
			VecType, "forward", &Vector3_Forward));

	EngineModule.addFunction(
		NativeFunction({ FunctionArgument(RotType, "rotation") },
			VecType, "right", &Vector3_Right));

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
	EngineInputModule.addEnumValue(KeyType, "space", Key::SPACE);
	EngineInputModule.addEnumValue(KeyType, "shift", Key::LSHIFT);
	EngineInputModule.addEnumValue(KeyType, "ctrl", Key::LCTRL);
	EngineInputModule.addEnumValue(KeyType, "enter", Key::RETURN);
	EngineInputModule.addFunction(NativeFunction({ FunctionArgument(KeyType, "toCheck") },
		BoolType::getInstance(), "isKeyDown", &Input_IsKeyDown));

	EngineInputModule.addFunction(NativeFunction({},
		BoolType::getInstance(), "isLMBDown", &Input_IsLMBDown));

	EngineInputModule.addFunction(NativeFunction({},
		BoolType::getInstance(), "isRMBDown", &Input_IsRMBDown));

	EngineInputModule.addFunction(NativeFunction({},
		BoolType::getInstance(), "isLMBClicked", &Input_IsLMBClicked));

	EngineInputModule.addFunction(NativeFunction({},
		BoolType::getInstance(), "isRMBClicked", &Input_IsRMBClicked));

	EngineInputModule.addFunction(NativeFunction({ },
		Vec2Type, "getMouseMovement", &Input_GetMouseMovement));

	ToContext->addNativeModule(EngineModule);
	ToContext->addNativeModule(EngineInputModule);
}

ds::RuntimeClass* engine::script::CreateAssetRef()
{
	ClassRef<AssetRef*> NewAssetRef = RuntimeClass::allocateClass(sizeof(AssetRef*), &AssetRef_vTable);
	NewAssetRef.classPtr->vtable = &AssetRef_vTable;

	NewAssetRef.getValue() = new AssetRef();
	return NewAssetRef.classPtr;
}

void engine::script::UpdateWaitTasks()
{
}
