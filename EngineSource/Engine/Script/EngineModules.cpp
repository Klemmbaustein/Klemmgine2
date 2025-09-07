#include "EngineModules.h"
#include <Engine/Objects/Components/MeshComponent.h>
#include <Engine/Objects/SceneObject.h>
#include <language.hpp>
#include <modules/system.hpp>
#include <native/nativeModule.hpp>
#include <parser/types/stringType.hpp>
#include <Core/Log.h>
#include <Engine/Script/ScriptObject.h>

using namespace lang;
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
	RuntimeStr File = context->popValue<RuntimeStr>();

	Component.getValue()->Load(AssetRef::FromName(File.ptr(), "kmdl"));
}

static void Log_Info(InterpretContext* context)
{
	auto message = context->popRuntimeString();

	Log::Info(std::string(message.ptr(), message.length()));
}

static void Log_Warn(InterpretContext* context)
{
	auto message = context->popRuntimeString();

	Log::Warn(std::string(message.ptr(), message.length()));
}

void engine::script::RegisterEngineModules(lang::LanguageContext* ToContext)
{
	NativeModule EngineModule;
	EngineModule.name = "engine";

	auto StrType = StringType::getInstance();

	auto ObjectType = EngineModule.createClass<ScriptObject>("SceneObject");
	auto ComponentType = EngineModule.createClass<ObjectComponent*>("ObjectComponent");

	EngineModule.addClassVirtualMethod(ObjectType,
		NativeFunction({}, nullptr, "begin", &SceneObject_empty), 1);

	EngineModule.addClassVirtualMethod(ObjectType,
		NativeFunction({}, nullptr, "destroy", &SceneObject_empty), 2);

	EngineModule.addClassVirtualMethod(ObjectType,
		NativeFunction({}, nullptr, "update", &SceneObject_empty), 3);

	EngineModule.addClassMethod(ObjectType,
		NativeFunction({FunctionArgument(ComponentType->nullable, "component")}, nullptr,
			"attach", &SceneObject_attach));

	EngineModule.addFunction(
		NativeFunction({FunctionArgument(StringType::getInstance(), "message")},
			nullptr, "info", &Log_Info));

	EngineModule.addFunction(
		NativeFunction({ FunctionArgument(StringType::getInstance(), "message") },
			nullptr, "warn", &Log_Warn));

	auto MeshComponentType = EngineModule.createClass<MeshComponent*>("MeshComponent", ComponentType);

	EngineModule.addClassConstructor(MeshComponentType,
		NativeFunction({ },
		nullptr, "MeshComponent.new", &MeshComponent_new));

	EngineModule.addClassMethod(MeshComponentType,
		NativeFunction({ FunctionArgument(StrType, "file")},
			nullptr, "load", &MeshComponent_load));

	EngineModule.attributes.push_back(new ExportAttribute());

	ToContext->addNativeModule(EngineModule);
}
