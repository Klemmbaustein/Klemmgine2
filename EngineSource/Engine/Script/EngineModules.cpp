#include "EngineModules.h"
#include <Engine/Objects/Components/MeshComponent.h>
#include <Engine/Objects/Components/MoveComponent.h>
#include <Engine/Objects/SceneObject.h>
#include <Engine/Input.h>
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

static void MoveComponent_new(InterpretContext* context)
{
	ClassRef<MoveComponent*> Component = context->popValue<RuntimeClass*>();
	Component.getValue() = new MoveComponent();
	context->pushValue(Component);
}

static void MeshComponent_load(InterpretContext* context)
{
	ClassRef<MeshComponent*> Component = context->popValue<RuntimeClass*>();
	ClassPtr<AssetRef*> File = context->popValue<RuntimeClass*>();

	Component.getValue()->Load(**File.get());
}

static void MoveComponent_addInput(InterpretContext* context)
{
	ClassRef<MoveComponent*> Component = context->popValue<RuntimeClass*>();
	Vector3 Direction = context->popValue<Vector3>();

	Component.getValue()->AddMovementInput(Direction);
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

static void Input_IsKeyDown(InterpretContext* context)
{
	context->pushValue<Bool>(IsKeyDown(context->popValue<Key>()));
}

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

	EngineModule.types.push_back(VecType);

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

	EngineModule.addClassMethod(ObjectType,
		NativeFunction({ FunctionArgument(ComponentType, "component") }, nullptr,
			"attach", &SceneObject_attach));

	EngineModule.addFunction(
		NativeFunction({ FunctionArgument(StringType::getInstance(), "message") },
			nullptr, "info", &Log_Info));

	EngineModule.addFunction(
		NativeFunction({ FunctionArgument(StringType::getInstance(), "message") },
			nullptr, "warn", &Log_Warn));

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

	EngineModule.addFunction(
		NativeFunction({ FunctionArgument(VecType, "message") },
			nullptr, "writeVec", &WriteVec));

	EngineModule.addFunction(
		NativeFunction({ FunctionArgument(FloatInst, "x"),FunctionArgument(FloatInst, "y"),FunctionArgument(FloatInst, "z") },
			VecType, "vec3", [](InterpretContext* context) {}));

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
	EngineInputModule.addFunction(NativeFunction({ FunctionArgument(KeyType, "toCheck") },
		BoolType::getInstance(), "isKeyDown", &Input_IsKeyDown));

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
