#include "PhysicsBindings.h"
#include <ds/parser/types/stringType.hpp>
#include <Core/Vector.h>
#include <Engine/Physics/Physics.h>
#include <Core/Transform.h>
#include <ds/language.hpp>

using namespace ds;
using namespace engine;

script::PhysicsBindings engine::script::AddPhysicsModule(ds::NativeModule& To, LanguageContext* ToContext)
{
	auto StrType = ToContext->registry.getEntry<StringType>();
	auto FloatInst = ToContext->registry.getEntry<FloatType>();
	auto BoolInst = ToContext->registry.getEntry<BoolType>();

	PhysicsBindings Physics;

	auto VecType = To.getType("Vector3");
	auto ObjectType = To.getType("SceneObject");

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
		.name = "hit",
		.offset = offsetof(physics::HitResult, Hit),
		.type = BoolInst
		});

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

	Physics.MotionTypeType = PhysicsMotionType;
	Physics.LayerType = PhysicsLayerType;
	Physics.HitResultType = HitResultType;

	return Physics;
}
