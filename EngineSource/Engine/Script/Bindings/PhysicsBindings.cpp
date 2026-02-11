#include "PhysicsBindings.h"
#include <ds/parser/types/stringType.hpp>
#include <Engine/Physics/Physics.h>
#include <ds/modules/system.hpp>
#include <Engine/Script/ScriptSubsystem.h>
#include <ds/parser/types/arrayType.hpp>
#include <ds/language.hpp>

using namespace ds;
using namespace engine;

using ds::modules::system::ArrayData;

static void Physics_rayCast(InterpretContext* context)
{
	ClassRef<physics::PhysicsManager*> Manager = context->popValue<RuntimeClass*>();

	ClassPtr<ArrayData> IgnoredObjects = context->popPtr<ArrayData>();
	physics::Layer Layer = physics::Layer(context->popValue<Int>());
	Vector3 End = context->popValue<Vector3>();
	Vector3 Start = context->popValue<Vector3>();

	std::set<SceneObject*> IgnoredSet;

	for (uint32_t i = 0; i < IgnoredObjects->length; i++)
	{
		IgnoredSet.insert(IgnoredObjects->at<SceneObject*>(i));
	}

	auto hit = Manager.getValue()->RayCast(Start, End, Layer, IgnoredSet);

	context->pushValue(hit.Hit ? NativeModule::makeClass(hit) : nullptr);
}

static void HitResult_getHitObject(InterpretContext* context)
{
	ClassRef<physics::HitResult> Hit = context->popValue<RuntimeClass*>();

	auto Obj = Hit->HitComponent->GetRootObject();

	auto Found = script::ScriptSubsystem::Instance->ScriptObjectMappings.find(Obj);

	if (Found != script::ScriptSubsystem::Instance->ScriptObjectMappings.end())
	{
		Found->second->addRef();
		context->pushValue<RuntimeClass*>(Found->second);
	}
	else
	{
		context->pushValue(nullptr);
	}
}

script::PhysicsBindings engine::script::AddPhysicsModule(ds::NativeModule& To, LanguageContext* ToContext)
{
	auto StrType = ToContext->registry->getEntry<StringType>();
	auto FloatInst = ToContext->registry->getEntry<FloatType>();
	auto BoolInst = ToContext->registry->getEntry<BoolType>();

	PhysicsBindings Physics;

	auto VecType = To.getType("Vector3");
	ClassType* ObjectType = static_cast<ClassType*>(To.getType("SceneObject"));

	auto PhysicsLayerType = To.createEnum("Layer");
	To.addEnumValue(PhysicsLayerType, "layer0", physics::Layer::Layer0);
	To.addEnumValue(PhysicsLayerType, "layer1", physics::Layer::Layer1);
	To.addEnumValue(PhysicsLayerType, "layer2", physics::Layer::Layer2);
	To.addEnumValue(PhysicsLayerType, "layer3", physics::Layer::Layer3);
	To.addEnumValue(PhysicsLayerType, "layer4", physics::Layer::Layer4);
	To.addEnumValue(PhysicsLayerType, "trigger", physics::Layer::Trigger);
	To.addEnumValue(PhysicsLayerType, "static", physics::Layer::Static);
	To.addEnumValue(PhysicsLayerType, "dynamic", physics::Layer::Dynamic);

	auto PhysicsMotionType = To.createEnum("MotionType");
	To.addEnumValue(PhysicsMotionType, "static", physics::MotionType::Static);
	To.addEnumValue(PhysicsMotionType, "kinematic", physics::MotionType::Kinematic);
	To.addEnumValue(PhysicsMotionType, "dynamic", physics::MotionType::Dynamic);

	auto HitResultType = To.createClass<physics::HitResult>("HitResult");

	HitResultType->members.push_back(ClassMember{
		.name = "depth",
		.offset = offsetof(physics::HitResult, Depth),
		.type = FloatInst
		});

	HitResultType->members.push_back(ClassMember{
		.name = "distance",
		.offset = offsetof(physics::HitResult, Distance),
		.type = FloatInst
		});

	HitResultType->members.push_back(ClassMember{
		.name = "impactPoint",
		.offset = offsetof(physics::HitResult, ImpactPoint),
		.type = VecType
		});

	HitResultType->members.push_back(ClassMember{
		.name = "normal",
		.offset = offsetof(physics::HitResult, Normal),
		.type = VecType
		});

	To.addClassMethod(HitResultType, NativeFunction({}, ObjectType->nullable, "getHitObject", HitResult_getHitObject));

	auto PhysicsManagerType = To.createClass<physics::PhysicsManager*>("PhysicsManager");

	To.addClassMethod(PhysicsManagerType, NativeFunction({
		FunctionArgument(VecType, "start"),
		FunctionArgument(VecType, "end"),
		FunctionArgument(PhysicsLayerType, "layer"),
		FunctionArgument(ToContext->registry->getArray(ObjectType), "ignoredObjects")
		}, HitResultType->nullable, "rayCast", &Physics_rayCast));

	PhysicsManagerType->members.push_back(ClassMember{
		.name = "isActive",
		.offset = offsetof(physics::PhysicsManager, Active),
		.type = BoolInst,
		});

	PhysicsManagerType->makePointerClass();

	Physics.MotionTypeType = PhysicsMotionType;
	Physics.LayerType = PhysicsLayerType;
	Physics.HitResultType = HitResultType;
	Physics.PhysicsManagerType = PhysicsManagerType;

	return Physics;
}
