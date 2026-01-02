#include "EngineModules.h"
#include <Engine/Objects/Components/MeshComponent.h>
#include <Engine/Objects/Components/MoveComponent.h>
#include <Engine/Objects/Components/CameraComponent.h>
#include <Engine/Objects/SceneObject.h>
#include <Engine/Script/ScriptSubsystem.h>
#include <Engine/Script/ScriptObject.h>
#include <Engine/Scene.h>
#include <Engine/Input.h>
#include <Engine/Engine.h>
#include <Engine/Stats.h>
#include <ds/language.hpp>
#include <ds/native/nativeGeneric.hpp>
#include <ds/native/nativeModule.hpp>
#include <ds/modules/system.hpp>
#include <ds/modules/system.async.hpp>
#include <ds/parser/types/stringType.hpp>
#include <ds/parser/types/taskType.hpp>
#include <ds/parser/types/arrayType.hpp>
#include <Core/Log.h>

#include "Bindings/MathBindings.h"

#define ENGINE_OFFSETOF(T, m)  (::std::size_t)&(reinterpret_cast<char const volatile&>(((T*)nullptr)->m))

using namespace ds;
using namespace engine::input;
using namespace engine;

class ExportAttribute : public modules::system::ReflectAttribute
{
public:
	ExportAttribute()
	{
		this->name = "Export";
		this->attributeParameters = { "name" };
	}
};

#pragma region Scene

static void Scene_new(InterpretContext* context)
{
	ClassRef<Scene*> NewScene = context->popValue<RuntimeClass*>();

	NewScene.getValue() = new Scene();
	context->pushValue(NewScene);
}

static void Scene_getMainScene(InterpretContext* context)
{
	Scene* Main = Scene::GetMain();

	if (Main)
	{
		ClassRef<Scene*> NewScene = RuntimeClass::allocateClass(sizeof(Scene*), nullptr);
		NewScene.getValue() = Main;
		context->pushValue(NewScene);
	}
	else
	{
		context->pushValue(nullptr);
	}
}

static void Scene_getObjects(InterpretContext* context)
{
	ClassRef<Scene*> TargetScene = context->popValue<RuntimeClass*>();

	std::vector<RuntimeClass*> FoundObjects;

	for (auto& i : TargetScene.getValue()->Objects)
	{
		FoundObjects.push_back(script::CreateSceneObject(i));
	}

	auto outArray = modules::system::createArray<RuntimeClass*>(FoundObjects.data(), FoundObjects.size(), true);
	context->pushValue(outArray);
}

static void Scene_getName(InterpretContext* context)
{
	ClassRef<Scene*> TargetScene = context->popValue<RuntimeClass*>();

	auto& Name = TargetScene.getValue()->Name;
	context->pushRuntimeString(RuntimeStr(Name.data(), Name.size()));
}

static void Scene_createNewObject(InterpretContext* context)
{
	auto obj = GenericData(context);

	ClassRef<Scene*> TargetScene = context->popValue<RuntimeClass*>();

	auto NewObject = TargetScene.getValue()->CreateObjectFromID(script::ScriptSubsystem::Instance->ScriptObjectIds.at(obj.id));

	auto ScriptObject = dynamic_cast<script::ScriptObject*>(NewObject);
	if (ScriptObject)
	{
		ScriptObject->ScriptData->addRef();
		context->pushValue(ScriptObject->ScriptData);
	}
	else
	{
		context->pushValue(script::CreateSceneObject(NewObject));
	}
}

#pragma endregion

#pragma region SceneObject

static void SceneObject_empty(InterpretContext* context)
{
	context->popValue<RuntimeClass*>();
}

static void SceneObject_attach(InterpretContext* context)
{
	ClassRef<SceneObject*> Data = context->popValue<RuntimeClass*>();
	ClassPtr<ObjectComponent*> Component = context->popPtr<ObjectComponent*>();
	Data.getValue()->Attach(*Component);
}

static void SceneObject_getScene(InterpretContext* context)
{
	ClassRef<SceneObject*> Data = context->popValue<RuntimeClass*>();

	Scene* FoundScene = Data.getValue()->GetScene();

	if (FoundScene)
	{
		ClassRef<Scene*> NewScene = RuntimeClass::allocateClass(sizeof(Scene*), nullptr);
		NewScene.getValue() = FoundScene;
		context->pushValue(NewScene);
	}
	else
	{
		context->pushValue(nullptr);
	}
}

static void SceneObject_getName(InterpretContext* context)
{
	ClassRef<SceneObject*> Data = context->popValue<RuntimeClass*>();

	std::string& Name = Data.getValue()->Name;

	context->pushRuntimeString(RuntimeStr(Name.data(), Name.size()));
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

static void MoveComponent_isOnGround(InterpretContext* context)
{
	ClassRef<MoveComponent*> Component = context->popValue<RuntimeClass*>();
	context->pushValue<Bool>(Component.getValue()->GetIsOnGround());
}

static void MoveComponent_setVelocity(InterpretContext* context)
{
	ClassRef<MoveComponent*> Component = context->popValue<RuntimeClass*>();
	Vector3 NewVelocity = context->popValue<Vector3>();
	Component.getValue()->SetVelocity(NewVelocity);
}

static void MoveComponent_getVelocity(InterpretContext* context)
{
	ClassRef<MoveComponent*> Component = context->popValue<RuntimeClass*>();
	context->pushValue(Component.getValue()->GetVelocity());
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

#pragma endregion

#pragma region Log

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

#pragma endregion

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

engine::script::EngineModuleData engine::script::RegisterEngineModules(ds::LanguageContext* ToContext)
{
	NativeModule EngineModule;
	EngineModule.name = "engine";

	auto StrType = ToContext->registry.getEntry<StringType>();
	auto FloatInst = ToContext->registry.getEntry<FloatType>();
	auto BoolInst = ToContext->registry.getEntry<BoolType>();

	MathBindings Math = AddMathModule(EngineModule, ToContext);

	auto AssetRefType = EngineModule.createClass<AssetRef*>("AssetRef");

	EngineModule.addClassConstructor(AssetRefType, NativeFunction{
		{FunctionArgument(StrType, "path")}, nullptr, "AssetRef.new",
		&AssetRef_new
		});

	auto SceneType = EngineModule.createClass<Scene*>("Scene");
	auto ObjectType = EngineModule.createClass<SceneObject*>("SceneObject");
	auto ComponentType = EngineModule.createClass<ObjectComponent*>("ObjectComponent");

	EngineModule.addClassConstructor(SceneType,
		NativeFunction({}, nullptr, "Scene.new", &Scene_new));

	EngineModule.addClassMethod(SceneType,
		NativeFunction({}, StrType, "getName", &Scene_getName));

	EngineModule.addClassMethod(SceneType,
		NativeGenericFunction({}, {GenericArgument("T", ObjectType)}, GenericArgumentType::getInstance(0, true), "createNewObject", &Scene_createNewObject));

	EngineModule.addClassMethod(SceneType,
		NativeFunction({}, ArrayType::getInstance(ObjectType), "getObjects", &Scene_getObjects));

	EngineModule.addFunction(NativeFunction({}, SceneType->nullable, "getMainScene", &Scene_getMainScene));

	ComponentType->members.push_back(ClassMember{
		.name = "position",
		.offset = offsetof(ObjectComponent, Position),
		.type = Math.Vec3
		});

	ComponentType->members.push_back(ClassMember{
		.name = "rotation",
		.offset = offsetof(ObjectComponent, Rotation),
		.type = Math.Rot
		});

	ComponentType->members.push_back(ClassMember{
		.name = "scale",
		.offset = offsetof(ObjectComponent, Scale),
		.type = Math.Vec3
		});

	ComponentType->makePointerClass();

	EngineModule.addClassVirtualMethod(ObjectType,
		NativeFunction({}, nullptr, "begin", &SceneObject_empty), 1);

	EngineModule.addClassVirtualMethod(ObjectType,
		NativeFunction({}, nullptr, "destroy", &SceneObject_empty), 2);

	EngineModule.addClassVirtualMethod(ObjectType,
		NativeFunction({}, nullptr, "update", &SceneObject_empty), 3);

	ObjectType->members.push_back(ClassMember{
		.name = "position",
		.offset = offsetof(SceneObject, Position),
		.type = Math.Vec3
		});

	ObjectType->members.push_back(ClassMember{
		.name = "rotation",
		.offset = offsetof(SceneObject, Rotation),
		.type = Math.Rot
		});

	ObjectType->members.push_back(ClassMember{
		.name = "scale",
		.offset = offsetof(SceneObject, Scale),
		.type = Math.Vec3
		});
	ObjectType->makePointerClass();

	EngineModule.addClassMethod(ObjectType,
		NativeFunction({ FunctionArgument(ComponentType, "component") }, nullptr,
			"attach", &SceneObject_attach));

	EngineModule.addClassMethod(ObjectType,
		NativeFunction({ }, SceneType->nullable,
			"getScene", &SceneObject_getScene));

	EngineModule.addClassMethod(ObjectType,
		NativeFunction({ }, StrType,
			"getName", &SceneObject_getName));

	EngineModule.addFunction(
		NativeFunction({ FunctionArgument(StrType, "message")},
			nullptr, "info", &Log_Info));

	EngineModule.addFunction(
		NativeFunction({ FunctionArgument(StrType, "message") },
			nullptr, "warn", &Log_Warn));

	EngineModule.addFunction(
		NativeFunction({ FunctionArgument(FloatInst, "timeInSeconds") },
			TaskType::getInstance(nullptr, &ToContext->registry), "wait", &Wait));

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
		NativeFunction({ FunctionArgument(Math.Vec3, "direction") },
			nullptr, "addInput", &MoveComponent_addInput));

	EngineModule.addClassMethod(MoveComponentType,
		NativeFunction({ },
			nullptr, "jump", &MoveComponent_jump));

	EngineModule.addClassMethod(MoveComponentType,
		NativeFunction({ },
			BoolInst, "isOnGround", &MoveComponent_isOnGround));

	EngineModule.addClassMethod(MoveComponentType,
		NativeFunction({ },
			Math.Vec3, "getVelocity", &MoveComponent_getVelocity));

	EngineModule.addClassMethod(MoveComponentType,
		NativeFunction({ FunctionArgument(Math.Vec3, "newVelocity") },
			nullptr, "setVelocity", &MoveComponent_setVelocity));

	MoveComponentType->members.push_back(ClassMember{
		.name = "acceleration",
		.offset = offsetof(MoveComponent, Acceleration),
		.type = FloatInst
		});

	MoveComponentType->members.push_back(ClassMember{
		.name = "gravity",
		.offset = offsetof(MoveComponent, Gravity),
		.type = FloatInst
		});
	MoveComponentType->members.push_back(ClassMember{
		.name = "maxSpeed",
		.offset = offsetof(MoveComponent, MaxSpeed),
		.type = FloatInst
		});
	MoveComponentType->members.push_back(ClassMember{
		.name = "deceleration",
		.offset = offsetof(MoveComponent, Deceleration),
		.type = FloatInst
		});
	MoveComponentType->members.push_back(ClassMember{
		.name = "active",
		.offset = offsetof(MoveComponent, Active),
		.type = BoolInst
		});
	MoveComponentType->members.push_back(ClassMember{
		.name = "canMoveUpSlopes",
		.offset = offsetof(MoveComponent, CanMoveUpSlopes),
		.type = BoolInst
		});

	MoveComponentType->makePointerClass();

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
		NativeFunction({ FunctionArgument(Math.Vec3, "message") },
			nullptr, "writeVec", &WriteVec));


	EngineModule.addFunction(
		NativeFunction({ FunctionArgument(StrType, "extension") },
			AssetRefType, "emptyAsset", AssetRef_emptyAsset));


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
		BoolInst, "isKeyDown", &Input_IsKeyDown));

	EngineInputModule.addFunction(NativeFunction({},
		BoolInst, "isLMBDown", &Input_IsLMBDown));

	EngineInputModule.addFunction(NativeFunction({},
		BoolInst, "isRMBDown", &Input_IsRMBDown));

	EngineInputModule.addFunction(NativeFunction({},
		BoolInst, "isLMBClicked", &Input_IsLMBClicked));

	EngineInputModule.addFunction(NativeFunction({},
		BoolInst, "isRMBClicked", &Input_IsRMBClicked));

	EngineInputModule.addFunction(NativeFunction({ },
		Math.Vec2, "getMouseMovement", &Input_GetMouseMovement));

	EngineModuleData OutData;
	OutData.ExportAttributeType = EngineModule.addAttribute(new ExportAttribute());

	ToContext->addNativeModule(EngineModule);
	ToContext->addNativeModule(EngineInputModule);

	OutData.Vector3Type = EngineModule.getType("Vector3")->id;
	OutData.ScriptObjectType = EngineModule.getType("SceneObject")->id;
	OutData.AssetRefType = EngineModule.getType("AssetRef")->id;

	return OutData;
}

ds::RuntimeClass* engine::script::CreateAssetRef()
{
	ClassRef<AssetRef*> NewAssetRef = RuntimeClass::allocateClass(sizeof(AssetRef*), &AssetRef_vTable);
	NewAssetRef.classPtr->vtable = &AssetRef_vTable;

	NewAssetRef.getValue() = new AssetRef();
	return NewAssetRef.classPtr;
}

ds::RuntimeClass* engine::script::CreateSceneObject(SceneObject* From)
{
	ClassRef<SceneObject*> NewObject = RuntimeClass::allocateClass(sizeof(SceneObject*), nullptr);
	NewObject.getValue() = From;
	return NewObject.classPtr;
}

void engine::script::UpdateWaitTasks()
{
}
