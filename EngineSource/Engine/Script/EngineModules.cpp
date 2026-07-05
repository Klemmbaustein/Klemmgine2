#include "EngineModules.h"
#include <Engine/Objects/Components/MeshComponent.h>
#include <Engine/Objects/Components/MoveComponent.h>
#include <Engine/Objects/Components/CameraComponent.h>
#include <Engine/Objects/Components/PhysicsComponent.h>
#include <Engine/Objects/Components/CollisionComponent.h>
#include <Engine/Objects/SceneObject.h>
#include <Engine/Script/ScriptSceneManager.h>
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
#include <ds/callableWrapper.hpp>
#include <Core/Log.h>
#include <kui/Window.h>
#include <ds/parser/types/functionType.hpp>
#include <Engine/Subsystem/SceneSubsystem.h>

#include "Bindings/MathBindings.h"
#include "Bindings/SerializeBindings.h"
#include "Bindings/PhysicsBindings.h"
#include "Bindings/AssetBindings.h"
#include "Bindings/SoundBindings.h"
#include "UI/UIBindings.h"

#ifdef EDITOR
#include <Editor/Editor.h>
#include <Editor/UI/Panels/Viewport.h>
#endif
#include <Engine/Objects/Components/SoundComponent.h>
#include <Engine/Objects/Components/BillboardComponent.h>
#include <Engine/Objects/Components/LightComponent.h>

#define CHECK_OBJ(obj) if (!obj.getValue()) { \
	context->runtimePanic(str::Format("Got null object: %s", __FUNCTION__).c_str()); \
	return; \
}

using namespace ds;
using namespace engine::input;
using namespace engine;
using namespace ds::modules::system;

class ExportAttribute : public ReflectAttribute
{
public:
	ExportAttribute()
	{
		this->name = "Export";
		this->attributeParameters = { "name", "visible", "hint" };
	}
};

#pragma region Scene

static void Scene_new(InterpretContext* context)
{
	ClassRef<Scene*> NewScene = context->popValue<RuntimeClass*>();

	NewScene.getValue() = new Scene();
	context->pushValue(NewScene);
}

static void engine_isPlaying(InterpretContext* context)
{
	context->pushValue<Bool>(Engine::IsPlaying);
}

static void Scene_setAsMain(InterpretContext* context)
{
	ClassRef<Scene*> NewScene = context->popValue<RuntimeClass*>();

	if (Engine::IsPlaying)
	{
		SceneSubsystem::Current->SetAsMain(NewScene.getValue());
	}
}

static void openScene(InterpretContext* context)
{
	auto Scene = context->popRuntimeString();
	if (Engine::IsPlaying)
	{
		SceneSubsystem::Current->LoadSceneAsync(string(Scene.ptr(), Scene.length()));
	}
}

static void Scene_getMainScene(InterpretContext* context)
{
	Scene* Main = Scene::GetMain();

	if (Main)
	{
		ClassRef<Scene*> NewScene = RuntimeClass::allocateClass(sizeof(Scene*), 0, nullptr);
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
		FoundObjects.push_back(script::ScriptSubsystem::Instance->GetClassFromObject(i));
	}

	auto outArray = createArray<RuntimeClass*>(FoundObjects.data(), FoundObjects.size(), true);
	context->pushValue(outArray);
}

static void Scene_getObjectByName(InterpretContext* context)
{
	auto obj = GenericData(context);

	auto found = script::ScriptSubsystem::Instance->ScriptObjectIds.find(obj.id);

	if (found == script::ScriptSubsystem::Instance->ScriptObjectIds.end())
	{
		context->runtimePanic("getObjectByName<T> was given an invalid type.");
		return;
	}

	auto& ReflectType = Reflection::ObjectTypes[found->second];

	ClassRef<Scene*> TargetScene = context->popValue<RuntimeClass*>();

	RuntimeStr Name = context->popRuntimeString();

	SceneObject* FoundObject = nullptr;

	for (auto& i : TargetScene.getValue()->Objects)
	{
		if (ReflectType.IsSubclassOf(i->TypeID) && i->Name == Name.ptr())
		{
			if (FoundObject)
			{
				Log::Warn("getObjectByName<T> found multiple matching objects, but can only return one.");
			}
			FoundObject = i;
		}
	}

	if (FoundObject)
	{
		context->pushValue(script::ScriptSubsystem::Instance->GetClassFromObject(FoundObject));
	}
	else
	{
		context->pushValue(nullptr);
	}
}

static void Scene_getPhysics(InterpretContext* context)
{
	ClassRef<Scene*> TargetScene = context->popValue<RuntimeClass*>();

	context->pushValue(NativeModule::makePointerClass(&TargetScene.getValue()->Physics));
}

static void Scene_getName(InterpretContext* context)
{
	ClassRef<Scene*> TargetScene = context->popValue<RuntimeClass*>();

	auto& Name = TargetScene.getValue()->Name;
	context->pushRuntimeString(RuntimeStr(Name.data(), Name.size()));
}

static void Scene_setName(InterpretContext* context)
{
	ClassRef<Scene*> TargetScene = context->popValue<RuntimeClass*>();
	auto NewName = context->popRuntimeString();

	TargetScene.getValue()->Name = string(NewName.ptr(), NewName.length());
}

static void Scene_getManager(InterpretContext* context)
{
	ClassRef<Scene*> TargetScene = context->popValue<RuntimeClass*>();

	if (TargetScene.getValue()->Manager)
	{
		context->pushValue(script::ScriptSubsystem::Instance->GetClassFromObject(TargetScene.getValue()->Manager));
	}
	else
	{
		context->pushValue(nullptr);
	}
}

static void Scene_getSoundContext(InterpretContext* context)
{
	ClassRef<Scene*> TargetScene = context->popValue<RuntimeClass*>();

	if (TargetScene.getValue()->Sound)
	{
		context->pushValue(NativeModule::makePointerClass(TargetScene.getValue()->Sound));
	}
	else
	{
		context->pushValue(nullptr);
	}
}

static void Scene_createNewObject(InterpretContext* context)
{
	auto obj = GenericData(context);

	ClassRef<Scene*> TargetScene = context->popValue<RuntimeClass*>();

	auto NewObject = TargetScene.getValue()->CreateObjectFromID(
		script::ScriptSubsystem::Instance->ScriptObjectIds.at(obj.id));

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

static void Object_empty(InterpretContext* context)
{
	context->popValue<RuntimeClass*>();
}

static RuntimeFunction SceneObject_vTable[] = {
	{},
	RuntimeFunction{.nativeFn = &Object_empty},
	RuntimeFunction{.nativeFn = &Object_empty},
	RuntimeFunction{.nativeFn = &Object_empty},
	RuntimeFunction{.nativeFn = &Object_empty},
};

static void SceneObject_destroy(InterpretContext* context)
{
	ClassRef<SceneObject*> obj = context->popValue<RuntimeClass*>();
	CHECK_OBJ(obj);
	obj.getValue()->Destroy();
}

static void SceneObject_attach(InterpretContext* context)
{
	ClassRef<SceneObject*> Data = context->popValue<RuntimeClass*>();
	CHECK_OBJ(Data);
	ClassPtr<ObjectComponent*> Component = context->popPtr<ObjectComponent*>();

	if ((*Component.get())->ParentObject || (*Component.get())->ParentComponent)
	{
		context->runtimePanic("Component attached twice");
		return;
	}

	Data.getValue()->Attach(*Component);
}

static void SceneObject_getScene(InterpretContext* context)
{
	ClassRef<SceneObject*> Data = context->popValue<RuntimeClass*>();
	CHECK_OBJ(Data);

	Scene* FoundScene = Data.getValue()->GetScene();

	if (FoundScene)
	{
		ClassRef<Scene*> NewScene = RuntimeClass::allocateClass(sizeof(Scene*), 0, nullptr);
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
	CHECK_OBJ(Data);

	const std::string& Name = Data.getValue()->Name;

	context->pushRuntimeString(RuntimeStr(Name.data(), Name.size()));
}

static void ObjectComponent_new(InterpretContext* context)
{
	// TODO: Add a vtable that frees this component if it has not been attached to anything.

	ClassRef<ObjectComponent*> Component = context->popValue<RuntimeClass*>();
	Component.getValue() = new ObjectComponent();
	context->pushValue(Component);
}

static void ObjectComponent_getWorldPosition(InterpretContext* context)
{
	ClassRef<ObjectComponent*> Component = context->popValue<RuntimeClass*>();

	context->pushValue(Component.getValue()->GetWorldTransform().ApplyTo(0));
}

static void ObjectComponent_getWorldTransform(InterpretContext* context)
{
	ClassRef<ObjectComponent*> Component = context->popValue<RuntimeClass*>();

	context->pushValue(Component.getValue()->GetWorldTransform());
}

static void ObjectComponent_attach(InterpretContext* context)
{
	ClassRef<ObjectComponent*> Component = context->popValue<RuntimeClass*>();
	ClassPtr<ObjectComponent*> SubComponent = context->popPtr<ObjectComponent*>();

	if ((*SubComponent.get())->ParentObject || (*SubComponent.get())->ParentComponent)
	{
		context->runtimePanic("Component attached twice");
		return;
	}

	Component.getValue()->Attach(*SubComponent);
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

static void MeshComponent_getModel(InterpretContext* context)
{
	ClassRef<MeshComponent*> Component = context->popValue<RuntimeClass*>();

	auto Model = Component.getValue()->DrawnModel;

	if (Model)
	{
		context->pushValue(script::CreateModelDataClass(Model, context));
	}
	else
	{
		context->pushValue(nullptr);
	}
}

static void DrawableComponent_getBounds(InterpretContext* context)
{
	ClassRef<DrawableComponent*> Component = context->popValue<RuntimeClass*>();

	context->pushValue(Component.getValue()->DrawBoundingBox);
}

static void PhysicsComponent_new(InterpretContext* context)
{
	ClassRef<PhysicsComponent*> Component = context->popValue<RuntimeClass*>();
	Component.getValue() = new PhysicsComponent();
	context->pushValue(Component);
}

static void PhysicsComponent_createBox(InterpretContext* context)
{
	ClassRef<PhysicsComponent*> Component = context->popValue<RuntimeClass*>();

	Vector3 Scale = context->popValue<Vector3>();
	physics::Layer Layer = physics::Layer(context->popValue<Int>());
	physics::MotionType MotionType = physics::MotionType(context->popValue<Int>());

	Component.getValue()->CreateBox(MotionType, Layer, Scale);
}

static void PhysicsComponent_createSphere(InterpretContext* context)
{
	ClassRef<PhysicsComponent*> Component = context->popValue<RuntimeClass*>();

	Float Scale = context->popValue<Float>();
	physics::Layer Layer = physics::Layer(context->popValue<Int>());
	physics::MotionType MotionType = context->popValue<physics::MotionType>();

	Component.getValue()->CreateSphere(MotionType, Layer, Scale);
}

static void PhysicsComponent_setIsActive(InterpretContext* context)
{
	ClassRef<PhysicsComponent*> Component = context->popValue<RuntimeClass*>();

	Bool Active = context->popValue<Bool>();

	Component.getValue()->SetActive(Active);
}

static void PhysicsComponent_setIsCollisionEnabled(InterpretContext* context)
{
	ClassRef<PhysicsComponent*> Component = context->popValue<RuntimeClass*>();

	Bool CollisionEnabled = context->popValue<Bool>();

	Component.getValue()->SetCollisionEnabled(CollisionEnabled);
}

static void PhysicsComponent_collisionTest(InterpretContext* context)
{
	ClassRef<PhysicsComponent*> Component = context->popValue<RuntimeClass*>();

	physics::Layer Layer = physics::Layer(context->popValue<Int>());

	auto Hit = Component.getValue()->CollisionTest(Layer);

	std::vector<ds::RuntimeClass*> Classes;

	for (auto& h : Hit)
	{
		Classes.push_back(NativeModule::makeClass(h));
	}

	context->pushValue(createArray(Classes.data(), Classes.size(), true));
}

static void PhysicsComponent_shapeCast(InterpretContext* context)
{
	ClassRef<PhysicsComponent*> Component = context->popValue<RuntimeClass*>();

	ClassPtr<ArrayData> IgnoredObjects = context->popPtr<ArrayData>();
	physics::Layer Layer = physics::Layer(context->popValue<Int>());
	Vector3 End = context->popValue<Vector3>();

	std::set<SceneObject*> IgnoredSet;

	for (uint32_t i = 0; i < IgnoredObjects->length; i++)
	{
		IgnoredSet.insert(*reinterpret_cast<SceneObject**>(IgnoredObjects->at<RuntimeClass*>(i)->getBody()));
	}

	auto Hit = Component.getValue()->ShapeCast(Component.getValue()->GetWorldTransform(), End,
		Layer, IgnoredSet);

	std::vector<RuntimeClass*> Classes;

	for (auto& h : Hit)
	{
		Classes.push_back(NativeModule::makeClass(h));
	}

	context->pushValue(createArray(Classes.data(), Classes.size(), true));
}

static void PhysicsComponent_onBeginOverlap(InterpretContext* context)
{
	ClassRef<PhysicsComponent*> Component = context->popValue<RuntimeClass*>();

	CallableWrapper<void> Callable = { context->popValue<RuntimeClass*>(), context };

	Component.getValue()->OnBeginOverlap = Callable;
}

static void CollisionComponent_new(InterpretContext* context)
{
	ClassRef<CollisionComponent*> Component = context->popValue<RuntimeClass*>();
	Component.getValue() = new CollisionComponent();
	context->pushValue(Component);
}

static void CollisionComponent_load(InterpretContext* context)
{
	ClassRef<CollisionComponent*> Component = context->popValue<RuntimeClass*>();
	ClassPtr<AssetRef*> File = context->popPtr<AssetRef*>();

	Component.getValue()->Load(**File);
}

static void CollisionComponent_setIsCollisionEnabled(InterpretContext* context)
{
	ClassRef<CollisionComponent*> Component = context->popValue<RuntimeClass*>();

	Bool CollisionEnabled = context->popValue<Bool>();

	Component.getValue()->SetCollisionEnabled(CollisionEnabled);
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

static void SoundComponent_new(InterpretContext* context)
{
	ClassRef<SoundComponent*> Component = context->popValue<RuntimeClass*>();
	Component.getValue() = new SoundComponent();
	context->pushValue(Component);
}

static void SoundComponent_load(InterpretContext* context)
{
	ClassRef<SoundComponent*> Component = context->popValue<RuntimeClass*>();
	ClassPtr<AssetRef*> Sound = context->popPtr<AssetRef*>();

	Component.getValue()->Load(**Sound);
}

static void SoundComponent_play(InterpretContext* context)
{
	ClassRef<SoundComponent*> Component = context->popValue<RuntimeClass*>();
	Bool Is3D = context->popValue<Bool>();
	Bool Loop = context->popValue<Bool>();

	Component.getValue()->Play(Loop, Is3D);
}

static void SoundComponent_stop(InterpretContext* context)
{
	ClassRef<SoundComponent*> Component = context->popValue<RuntimeClass*>();

	Component.getValue()->Stop();
}

static void SoundComponent_setVolume(InterpretContext* context)
{
	ClassRef<SoundComponent*> Component = context->popValue<RuntimeClass*>();
	Float Volume = context->popValue<Float>();

	Component.getValue()->SetVolume(Volume);
}

static void SoundComponent_setPitch(InterpretContext* context)
{
	ClassRef<SoundComponent*> Component = context->popValue<RuntimeClass*>();
	Float Pitch = context->popValue<Float>();

	Component.getValue()->SetPitch(Pitch);
}

static void SoundComponent_setRange(InterpretContext* context)
{
	ClassRef<SoundComponent*> Component = context->popValue<RuntimeClass*>();
	Float Range = context->popValue<Float>();

	Component.getValue()->SetRange(Range);
}

static void BillboardComponent_new(InterpretContext* context)
{
	ClassRef<BillboardComponent*> Component = context->popValue<RuntimeClass*>();
	Component.getValue() = new BillboardComponent();
	context->pushValue(Component);
}

static void BillboardComponent_load(InterpretContext* context)
{
	ClassRef<BillboardComponent*> Component = context->popValue<RuntimeClass*>();
	ClassPtr<AssetRef*> Image = context->popPtr<AssetRef*>();

	Component.getValue()->LoadImage(**Image);
}

static void BillboardComponent_setColor(InterpretContext* context)
{
	ClassRef<BillboardComponent*> Component = context->popValue<RuntimeClass*>();
	Vector3 Color = context->popValue<Vector3>();

	Component.getValue()->SetColor(Color);
}

static void LightComponent_new(InterpretContext* context)
{
	ClassRef<LightComponent*> Component = context->popValue<RuntimeClass*>();
	Component.getValue() = new LightComponent();
	context->pushValue(Component);
}

static void LightComponent_setColor(InterpretContext* context)
{
	ClassRef<LightComponent*> Component = context->popValue<RuntimeClass*>();
	Vector3 Color = context->popValue<Vector3>();

	Component.getValue()->SetColor(Color);
}

static void LightComponent_setIntensity(InterpretContext* context)
{
	ClassRef<LightComponent*> Component = context->popValue<RuntimeClass*>();
	Float Intensity = context->popValue<Float>();

	Component.getValue()->SetIntensity(Intensity);
}

static void LightComponent_setRange(InterpretContext* context)
{
	ClassRef<LightComponent*> Component = context->popValue<RuntimeClass*>();
	Float Intensity = context->popValue<Float>();

	Component.getValue()->SetRange(Intensity);
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

static void CameraComponent_screenToWorldDirection(InterpretContext* context)
{
	ClassRef<CameraComponent*> Component = context->popValue<RuntimeClass*>();

	Vector2 Screen = context->popValue<Vector2>();
	context->pushValue(Component.getValue()->ScreenToWorld(Screen));
}

static void CameraComponent_worldPositionToScreen(InterpretContext* context)
{
	ClassRef<CameraComponent*> Component = context->popValue<RuntimeClass*>();

	Vector3 World = context->popValue<Vector3>();
	context->pushValue(Component.getValue()->WorldToScreen(World));
}

#pragma endregion

#pragma region Log

static void Log_info(InterpretContext* context)
{
	auto message = context->popRuntimeString();

	Log::Info(string(message.ptr(), message.length()));
}

static void Log_warn(InterpretContext* context)
{
	auto message = context->popRuntimeString();

	Log::Warn(string(message.ptr(), message.length()));
}

#pragma endregion

static void Stats_getDelta(InterpretContext* context)
{
	context->pushValue(stats::DeltaTime);
}

#pragma region Input

static void Input_isKeyHeld(InterpretContext* context)
{
	context->pushValue<Bool>(IsKeyHeld(context->popValue<Key>()));
}

static void Input_isKeyDown(InterpretContext* context)
{
	context->pushValue<Bool>(IsKeyPressed(context->popValue<Key>()));
}

static void Input_isKeyUp(InterpretContext* context)
{
	context->pushValue<Bool>(IsKeyReleased(context->popValue<Key>()));
}

static void Input_getMouseMovement(InterpretContext* context)
{
	context->pushValue<Vector2>(MouseMovement);
}

static void Input_getMousePosition(InterpretContext* context)
{
#ifdef EDITOR
	if (editor::IsActive())
	{
		context->pushValue<kui::Vec2f>(editor::Viewport::GetMousePositionViewportRelative());
	}
	else
#endif
	{
		context->pushValue<kui::Vec2f>(kui::Window::GetActiveWindow()->Input.MousePosition);
	}
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

static void Input_setShowMouseCursor(InterpretContext* context)
{
	bool Value = context->popValue<Bool>();

	if (Engine::GameHasFocus)
	{
		input::ShowMouseCursor = Value;
	}
}

#pragma endregion

static void Vector3_Length(InterpretContext* context)
{
	context->pushValue(context->popValue<Vector3>().Length());
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

engine::script::EngineModuleData engine::script::RegisterEngineModules(LanguageContext* ToContext)
{
	NativeModule EngineModule;
	EngineModule.name = "engine";

	auto StrType = ToContext->registry->getEntry<StringType>();
	auto FloatInst = ToContext->registry->getEntry<FloatType>();
	auto BoolInst = ToContext->registry->getEntry<BoolType>();

	auto SceneType = EngineModule.createClass<Scene*>("Scene");
	auto ObjectType = EngineModule.createClass<SceneObject*>("SceneObject");
	auto ManagerType = EngineModule.createClass<SceneManager*>("SceneManager");
	auto ComponentType = EngineModule.createClass<ObjectComponent*>("ObjectComponent");

	NativeModule EngineUIModule;
	EngineUIModule.name = "engine::ui";

	MathBindings Math = AddMathModule(EngineModule, ToContext);
	AssetBindings Assets = AddAssetBindings(EngineModule, ToContext);
	SerializeBindings Serialize = AddSerializeModule(EngineModule, ToContext);
	ui::UIBindings UI = ui::AddUIModule(EngineUIModule, EngineModule, ToContext);
	PhysicsBindings Physics = AddPhysicsModule(EngineModule, ToContext);
	SoundBindings Sound = AddSoundModule(EngineModule, ToContext);

	EngineModule.addClassConstructor(SceneType,
		NativeFunction({}, nullptr, "Scene.new", &Scene_new));

	EngineModule.addClassMethod(SceneType,
		NativeFunction({}, StrType, "getName", &Scene_getName));

	EngineModule.addClassMethod(SceneType,
		NativeFunction({}, ManagerType->nullable, "getManager", &Scene_getManager));

	EngineModule.addClassMethod(SceneType,
		NativeFunction({ FunctionArgument(StrType, "newName") }, nullptr, "setName", &Scene_setName));

	EngineModule.addClassMethod(SceneType,
		NativeFunction({ }, Sound.SoundContext, "getSoundContext", &Scene_getSoundContext));

	EngineModule.addClassMethod(SceneType,
		NativeFunction({ }, nullptr, "setAsMain", &Scene_setAsMain));

	EngineModule.addClassMethod(SceneType,
		NativeGenericFunction({}, { GenericArgument("T", ObjectType) },
			GenericArgumentType::getInstance(0, true), "createNewObject", &Scene_createNewObject));

	EngineModule.addClassMethod(SceneType,
		NativeFunction({}, ToContext->registry->getArray(ObjectType), "getObjects", &Scene_getObjects));

	EngineModule.addClassMethod(SceneType,
		NativeGenericFunction({ FunctionArgument(StrType, "name") }, { GenericArgument("T", ObjectType) },
			GenericArgumentType::getInstance(0, true)->nullable, "getObjectByName", &Scene_getObjectByName));

	EngineModule.addClassMethod(SceneType,
		NativeFunction({}, Physics.PhysicsManagerType, "getPhysics", &Scene_getPhysics));

	EngineModule.addFunction(NativeFunction({}, SceneType->nullable, "getMainScene", &Scene_getMainScene));
	EngineModule.addFunction(NativeFunction({ FunctionArgument(StrType, "name") }, nullptr, "openScene", &openScene));

	ComponentType->members.push_back(ClassMember{
		.name = "position",
		.offset = DS_OFFSETOF(ObjectComponent, Position),
		.type = Math.Vec3
		});

	ComponentType->members.push_back(ClassMember{
		.name = "rotation",
		.offset = DS_OFFSETOF(ObjectComponent, Rotation),
		.type = Math.Rot
		});

	ComponentType->members.push_back(ClassMember{
		.name = "scale",
		.offset = DS_OFFSETOF(ObjectComponent, Scale),
		.type = Math.Vec3
		});

	EngineModule.addClassConstructor(ComponentType,
		NativeFunction({ }, nullptr,
			"ObjectComponent.new", &ObjectComponent_new));

	EngineModule.addClassMethod(ComponentType,
		NativeFunction({ FunctionArgument(ComponentType, "component") }, nullptr,
			"attach", &ObjectComponent_attach));

	ComponentType->makePointerClass();

	EngineModule.addClassVirtualMethod(ManagerType,
		NativeFunction({}, nullptr, "update", &Object_empty), 1);

	EngineModule.addClassVirtualMethod(ObjectType,
		NativeFunction({}, nullptr, "onBegin", &Object_empty), 1);

	EngineModule.addClassVirtualMethod(ObjectType,
		NativeFunction({}, nullptr, "onBeginPlay", &Object_empty), 2);

	EngineModule.addClassVirtualMethod(ObjectType,
		NativeFunction({}, nullptr, "onDestroy", &Object_empty), 3);

	EngineModule.addClassVirtualMethod(ObjectType,
		NativeFunction({}, nullptr, "update", &Object_empty), 4);

	ObjectType->members.push_back(ClassMember{
		.name = "position",
		.offset = DS_OFFSETOF(SceneObject, Position),
		.type = Math.Vec3
		});

	ObjectType->members.push_back(ClassMember{
		.name = "rotation",
		.offset = DS_OFFSETOF(SceneObject, Rotation),
		.type = Math.Rot
		});

	ObjectType->members.push_back(ClassMember{
		.name = "scale",
		.offset = DS_OFFSETOF(SceneObject, Scale),
		.type = Math.Vec3
		});

	ObjectType->members.push_back(ClassMember{
		.name = "objectTransform",
		.offset = DS_OFFSETOF(SceneObject, ObjectTransform),
		.type = Math.Transform
		});

	ObjectType->allowDirectConstructorCall = false;
	ObjectType->makePointerClass();
	ManagerType->makePointerClass();

	EngineModule.addClassMethod(ObjectType,
		NativeFunction({ FunctionArgument(ComponentType, "component") }, nullptr,
			"attach", &SceneObject_attach));

	EngineModule.addClassMethod(ObjectType,
		NativeFunction({ }, nullptr,
			"destroy", &SceneObject_destroy));

	EngineModule.addClassMethod(ObjectType,
		NativeFunction({ }, SceneType->nullable,
			"getScene", &SceneObject_getScene));

	EngineModule.addClassMethod(ObjectType,
		NativeFunction({ }, StrType,
			"getName", &SceneObject_getName));

	EngineModule.addFunction(
		NativeFunction({ FunctionArgument(StrType, "message") },
			nullptr, "info", &Log_info));

	EngineModule.addFunction(
		NativeFunction({ FunctionArgument(StrType, "message") },
			nullptr, "warn", &Log_warn));

	EngineModule.addFunction(
		NativeFunction({ FunctionArgument(FloatInst, "timeInSeconds") },
			TaskType::getInstance(nullptr, ToContext->registry), "wait", &Wait));

	EngineModule.addFunction(
		NativeFunction({ },
			FloatInst, "getDelta", &Stats_getDelta));

	EngineModule.addClassMethod(ComponentType, NativeFunction({}, Math.Vec3, "getWorldPosition",
		&ObjectComponent_getWorldPosition));

	EngineModule.addClassMethod(ComponentType, NativeFunction({}, Math.Transform, "getWorldTransform",
		&ObjectComponent_getWorldTransform));

	auto DrawableComponentType = EngineModule.createClass<DrawableComponent*>("DrawableComponent", ComponentType);

	DrawableComponentType->members.push_back(ClassMember{
		.name = "isVisible",
		.offset = DS_OFFSETOF(DrawableComponent, IsVisible),
		.type = BoolInst,
		});

	DrawableComponentType->members.push_back(ClassMember{
		.name = "castShadow",
		.offset = DS_OFFSETOF(DrawableComponent, CastShadow),
		.type = BoolInst,
		});

	auto MeshComponentType = EngineModule.createClass<MeshComponent*>("MeshComponent", DrawableComponentType);

	EngineModule.addClassConstructor(MeshComponentType,
		NativeFunction({ },
			nullptr, "MeshComponent.new", &MeshComponent_new));

	EngineModule.addClassMethod(MeshComponentType,
		NativeFunction({ FunctionArgument(Assets.AssetRef, "file") },
			nullptr, "load", &MeshComponent_load));

	EngineModule.addClassMethod(MeshComponentType,
		NativeFunction({},
			Assets.ModelData->nullable, "getModel", &MeshComponent_getModel));


	auto PhysicsComponentType = EngineModule.createClass<PhysicsComponent*>("PhysicsComponent", ComponentType);

	EngineModule.addClassConstructor(PhysicsComponentType,
		NativeFunction({}, nullptr, "PhysicsComponent.new",
			&PhysicsComponent_new));

	EngineModule.addClassMethod(PhysicsComponentType,
		NativeFunction(
			{ FunctionArgument(Physics.MotionTypeType, "motionType"), FunctionArgument(Physics.LayerType, "layer"),
			FunctionArgument(Math.Vec3, "scale") },
			nullptr, "createBox",
			&PhysicsComponent_createBox));

	EngineModule.addClassMethod(PhysicsComponentType,
		NativeFunction(
			{ FunctionArgument(Physics.MotionTypeType, "motionType"), FunctionArgument(Physics.LayerType, "layer"),
			FunctionArgument(FloatInst, "scale") },
			nullptr, "createSphere",
			&PhysicsComponent_createSphere));

	EngineModule.addClassMethod(PhysicsComponentType,
		NativeFunction(
			{ FunctionArgument(Math.Vec3, "targetPosition"), FunctionArgument(Physics.LayerType, "layer"),
			FunctionArgument(ToContext->registry->getArray(ObjectType), "ignoredObjects") },
			ToContext->registry->getArray(Physics.HitResultType), "shapeCast",
			&PhysicsComponent_shapeCast));

	EngineModule.addClassMethod(PhysicsComponentType,
		NativeFunction(
			{ FunctionArgument(BoolInst, "isActive") },
			nullptr, "setIsActive",
			&PhysicsComponent_setIsActive));

	EngineModule.addClassMethod(PhysicsComponentType,
		NativeFunction(
			{ FunctionArgument(BoolInst, "isCollisionEnabled") },
			nullptr, "setIsCollisionEnabled",
			&PhysicsComponent_setIsCollisionEnabled));

	EngineModule.addClassMethod(PhysicsComponentType,
		NativeFunction(
			{ FunctionArgument(Physics.LayerType, "layer") },
			ToContext->registry->getArray(Physics.HitResultType), "collisionTest",
			&PhysicsComponent_collisionTest));

	EngineModule.addClassMethod(PhysicsComponentType,
		NativeFunction(
			{ FunctionArgument(FunctionType::getInstance(nullptr, {}, ToContext->registry), "onOverlapped") },
			nullptr, "onBeginOverlap",
			&PhysicsComponent_onBeginOverlap));

	auto CollisionComponentType = EngineModule.createClass<PhysicsComponent*>("CollisionComponent", ComponentType);

	EngineModule.addClassConstructor(CollisionComponentType, NativeFunction({}, nullptr,
		"CollisionComponent.new", &CollisionComponent_new));

	EngineModule.addClassMethod(CollisionComponentType,
		NativeFunction({ FunctionArgument(Assets.AssetRef, "mesh") }, nullptr,
			"load", &CollisionComponent_load));

	EngineModule.addClassMethod(CollisionComponentType,
		NativeFunction(
			{ FunctionArgument(BoolInst, "isCollisionEnabled") },
			nullptr, "setIsCollisionEnabled",
			&CollisionComponent_setIsCollisionEnabled));


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

	auto SoundComponentType = EngineModule.createClass<SoundComponent*>("SoundComponent", ComponentType);

	EngineModule.addClassMethod(SoundComponentType, NativeFunction({ FunctionArgument(Assets.AssetRef, "soundFile") },
		nullptr, "load", &SoundComponent_load));

	EngineModule.addClassMethod(SoundComponentType, NativeFunction({ FunctionArgument(BoolInst, "loop"), FunctionArgument(BoolInst, "is3D") },
		nullptr, "play", &SoundComponent_play));

	EngineModule.addClassMethod(SoundComponentType, NativeFunction({ },
		nullptr, "stop", &SoundComponent_stop));

	EngineModule.addClassMethod(SoundComponentType, NativeFunction({ FunctionArgument(FloatInst, "newVolume") },
		nullptr, "setVolume", &SoundComponent_setVolume));

	EngineModule.addClassMethod(SoundComponentType, NativeFunction({ FunctionArgument(FloatInst, "newPitch") },
		nullptr, "setPitch", &SoundComponent_setPitch));

	EngineModule.addClassMethod(SoundComponentType, NativeFunction({ FunctionArgument(FloatInst, "newRange") },
		nullptr, "setRange", &SoundComponent_setRange));

	EngineModule.addClassConstructor(SoundComponentType, NativeFunction({},
		nullptr, "SoundComponent.new", &SoundComponent_new));

	auto BillboardComponentType = EngineModule.createClass<BillboardComponent*>("BillboardComponent", DrawableComponentType);

	EngineModule.addClassMethod(BillboardComponentType, NativeFunction({ FunctionArgument(Assets.AssetRef, "image") },
		nullptr, "load", &BillboardComponent_load));

	EngineModule.addClassMethod(BillboardComponentType, NativeFunction({ FunctionArgument(Math.Vec3, "newColor") },
		nullptr, "setColor", &BillboardComponent_setColor));

	EngineModule.addClassConstructor(BillboardComponentType, NativeFunction({},
		nullptr, "BillboardComponent.new", &BillboardComponent_new));

	auto LightComponentType = EngineModule.createClass<LightComponent*>("LightComponent", ComponentType);

	EngineModule.addClassMethod(LightComponentType, NativeFunction({ FunctionArgument(FloatInst, "newIntensity") },
		nullptr, "setIntensity", &LightComponent_setIntensity));

	EngineModule.addClassMethod(LightComponentType, NativeFunction({ FunctionArgument(FloatInst, "newRange") },
		nullptr, "setRange", &LightComponent_setRange));

	EngineModule.addClassMethod(LightComponentType, NativeFunction({ FunctionArgument(Math.Vec3, "newColor") },
		nullptr, "setColor", &LightComponent_setColor));

	EngineModule.addClassConstructor(LightComponentType, NativeFunction({},
		nullptr, "LightComponent.new", &LightComponent_new));

#ifdef EDITOR
	EngineModule.addConstant<Bool>("WITH_EDITOR", BoolInst, true);
#else
	EngineModule.addConstant<Bool>("WITH_EDITOR", BoolInst, false);
#endif

	MoveComponentType->members.push_back(ClassMember{
		.name = "acceleration",
		.offset = DS_OFFSETOF(MoveComponent, Acceleration),
		.type = FloatInst
		});

	MoveComponentType->members.push_back(ClassMember{
		.name = "gravity",
		.offset = DS_OFFSETOF(MoveComponent, Gravity),
		.type = FloatInst
		});
	MoveComponentType->members.push_back(ClassMember{
		.name = "maxSpeed",
		.offset = DS_OFFSETOF(MoveComponent, MaxSpeed),
		.type = FloatInst
		});
	MoveComponentType->members.push_back(ClassMember{
		.name = "deceleration",
		.offset = DS_OFFSETOF(MoveComponent, Deceleration),
		.type = FloatInst
		});
	MoveComponentType->members.push_back(ClassMember{
		.name = "active",
		.offset = DS_OFFSETOF(MoveComponent, Active),
		.type = BoolInst
		});
	MoveComponentType->members.push_back(ClassMember{
		.name = "canMoveUpSlopes",
		.offset = DS_OFFSETOF(MoveComponent, CanMoveUpSlopes),
		.type = BoolInst
		});
	MoveComponentType->members.push_back(ClassMember{
		.name = "jumpHeight",
		.offset = DS_OFFSETOF(MoveComponent, JumpHeight),
		.type = FloatInst
		});

	MeshComponentType->makePointerClass();
	PhysicsComponentType->makePointerClass();
	MoveComponentType->makePointerClass();
	DrawableComponentType->makePointerClass();

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

	EngineModule.addClassMethod(CameraComponentType,
		NativeFunction({ FunctionArgument(Math.Vec2, "screen") },
			Math.Vec3, "screenToWorldDirection", &CameraComponent_screenToWorldDirection));

	EngineModule.addClassMethod(CameraComponentType,
		NativeFunction({ FunctionArgument(Math.Vec3, "worldPosition") },
			Math.Vec3, "worldPositionToScreen", &CameraComponent_worldPositionToScreen));

	EngineModule.addFunction(
		NativeFunction({ },
			BoolInst, "isPlaying", &engine_isPlaying));

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
	EngineInputModule.addEnumValue(KeyType, "shift", Key::SHIFT);
	EngineInputModule.addEnumValue(KeyType, "ctrl", Key::CTRL);
	EngineInputModule.addEnumValue(KeyType, "enter", Key::RETURN);
	EngineInputModule.addEnumValue(KeyType, "backspace", Key::BACKSPACE);
	EngineInputModule.addEnumValue(KeyType, "escape", Key::ESCAPE);

	EngineInputModule.addFunction(NativeFunction({ FunctionArgument(KeyType, "toCheck") },
		BoolInst, "isKeyDown", &Input_isKeyDown));

	EngineInputModule.addFunction(NativeFunction({ FunctionArgument(KeyType, "toCheck") },
		BoolInst, "isKeyHeld", &Input_isKeyHeld));
	EngineInputModule.addFunction(NativeFunction({ FunctionArgument(KeyType, "toCheck") },
		BoolInst, "isKeyUp", &Input_isKeyUp));

	EngineInputModule.addFunction(NativeFunction({},
		BoolInst, "isLMBDown", &Input_IsLMBDown));

	EngineInputModule.addFunction(NativeFunction({},
		BoolInst, "isRMBDown", &Input_IsRMBDown));

	EngineInputModule.addFunction(NativeFunction({},
		BoolInst, "isLMBClicked", &Input_IsLMBClicked));

	EngineInputModule.addFunction(NativeFunction({},
		BoolInst, "isRMBClicked", &Input_IsRMBClicked));

	EngineInputModule.addFunction(NativeFunction({ },
		Math.Vec2, "getMouseMovement", &Input_getMouseMovement));

	EngineInputModule.addFunction(NativeFunction({ },
		Math.Vec2, "getMousePosition", &Input_getMousePosition));

	EngineInputModule.addFunction(NativeFunction({ FunctionArgument(BoolInst, "newValue") }, nullptr,
		"setShowMouseCursor", &Input_setShowMouseCursor));

	EngineModuleData OutData;
	OutData.ExportAttributeType = EngineModule.addAttribute(new ExportAttribute());

	ToContext->addNativeModule(EngineModule);
	ToContext->addNativeModule(EngineInputModule);
	ToContext->addNativeModule(EngineUIModule);

	OutData.Vector3Type = EngineModule.getType("Vector3")->id;
	OutData.SceneObjectType = EngineModule.getType("SceneObject")->id;
	OutData.SceneManagerType = EngineModule.getType("SceneManager")->id;
	OutData.AssetRefType = EngineModule.getType("AssetRef")->id;
	OutData.UITextType = EngineUIModule.getType("UIText")->id;

	return OutData;
}

ds::RuntimeClass* engine::script::CreateSceneObject(ReflectionObject* From)
{
	ClassRef<ReflectionObject*> NewObject = RuntimeClass::allocateClass(sizeof(ReflectionObject*),
		0, SceneObject_vTable);
	NewObject.getValue() = From;
	return NewObject.classPtr;
}

void engine::script::UpdateWaitTasks()
{
}
