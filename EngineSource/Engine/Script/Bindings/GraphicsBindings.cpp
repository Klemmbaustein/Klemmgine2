#include "GraphicsBindings.h"
#include <Engine/Graphics/Scene/Environment.h>
#include <ds/language.hpp>

using namespace engine;
using namespace engine::script;
using namespace ds;

GraphicsBindings engine::script::AddGraphicsModule(ds::NativeModule& To, ds::LanguageContext* ToContext)
{
	GraphicsBindings Out;
	auto Vec3Type = To.getType("Vector3");
	auto Rot3Type = To.getType("Rotation3");
	auto FloatInst = ToContext->registry->getEntry<FloatType>();

	NativeModule Graphics;
	Graphics.name = "engine::graphics";

	Out.EnvironmentType = Graphics.createClass<graphics::Environment*>("Environment");

	Out.EnvironmentType->members.push_back(ClassMember{
		.name = "sunColor",
		.offset = DS_OFFSETOF(graphics::Environment, SunColor),
		.type = Vec3Type,
		});

	Out.EnvironmentType->members.push_back(ClassMember{
		.name = "sunRotation",
		.offset = DS_OFFSETOF(graphics::Environment, SunRotation),
		.type = Rot3Type,
		});

	Out.EnvironmentType->members.push_back(ClassMember{
		.name = "skyColor",
		.offset = DS_OFFSETOF(graphics::Environment, SkyColor),
		.type = Vec3Type,
		});

	Out.EnvironmentType->members.push_back(ClassMember{
		.name = "groundColor",
		.offset = DS_OFFSETOF(graphics::Environment, GroundColor),
		.type = Vec3Type,
		});

	Out.EnvironmentType->members.push_back(ClassMember{
		.name = "fogColor",
		.offset = DS_OFFSETOF(graphics::Environment, FogColor),
		.type = Vec3Type,
		});

	Out.EnvironmentType->members.push_back(ClassMember{
		.name = "fogRange",
		.offset = DS_OFFSETOF(graphics::Environment, FogRange),
		.type = FloatInst,
		});

	Out.EnvironmentType->members.push_back(ClassMember{
		.name = "fogStart",
		.offset = DS_OFFSETOF(graphics::Environment, FogStart),
		.type = FloatInst,
		});

	Out.EnvironmentType->makePointerClass();

	ToContext->addNativeModule(Graphics);

	return Out;
}
