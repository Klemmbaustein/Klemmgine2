#include "SoundBindings.h"
#include <Engine/Sound/Sound.h>
#include <Engine/Sound/SoundSubsystem.h>
#include <Engine/Engine.h>
#include <ds/language.hpp>

using namespace engine::script;
using namespace engine::sound;
using namespace engine;
using namespace ds;

static void Sound_getMainSoundContext(InterpretContext* context)
{
	auto sys = Engine::GetSubsystem<SoundSubsystem>();

	if (sys && sys->MainContext)
	{
		context->pushValue(NativeModule::makePointerClass(sys->MainContext));
	}
	else
	{
		context->pushValue(nullptr);
	}
}

static void SoundContext_playSound(InterpretContext* context)
{
	ClassRef<SoundContext*> Context = context->popValue<RuntimeClass*>();
	Float Pitch = context->popValue<Float>();
	Float Volume = context->popValue<Float>();
	ClassPtr<AssetRef*> Sound = context->popPtr<AssetRef*>();

	Context.getValue()->PlaySound((*Sound)->FilePath, Volume, Pitch);
}

static void SoundContext_playSoundAt(InterpretContext* context)
{
	ClassRef<SoundContext*> Context = context->popValue<RuntimeClass*>();
	Float Falloff = context->popValue<Float>();
	Vector3 Position = context->popValue<Vector3>();
	Float Pitch = context->popValue<Float>();
	Float Volume = context->popValue<Float>();
	ClassPtr<AssetRef*> Sound = context->popPtr<AssetRef*>();

	Context.getValue()->PlaySoundAt((*Sound)->FilePath, Volume, Pitch, Position, Falloff);
}

SoundBindings engine::script::AddSoundModule(ds::NativeModule& To, ds::LanguageContext* ToContext)
{
	NativeModule Module;
	Module.name = "engine::sound";
	auto AssetType = To.getType("AssetRef");
	auto VecType = To.getType("Vector3");
	auto StrType = ToContext->registry->getEntry<StringType>();
	auto FloatInst = ToContext->registry->getEntry<FloatType>();

	SoundBindings Sound;

	Sound.SoundContext = Module.createClass<SoundContext*>("SoundContext");

	Module.addClassMethod(Sound.SoundContext, NativeFunction({ FunctionArgument(AssetType, "sound"),
		FunctionArgument(FloatInst, "volume"), FunctionArgument(FloatInst, "pitch") },
		nullptr, "playSound", &SoundContext_playSound));

	Module.addClassMethod(Sound.SoundContext, NativeFunction({ FunctionArgument(AssetType, "sound"),
		FunctionArgument(FloatInst, "volume"), FunctionArgument(FloatInst, "pitch"),
		FunctionArgument(VecType, "position"), FunctionArgument(FloatInst, "falloff") },
		nullptr, "playSoundAt", &SoundContext_playSoundAt));

	Module.addFunction(NativeFunction({},
		Sound.SoundContext->nullable, "getMainSoundContext", &Sound_getMainSoundContext));

	ToContext->addNativeModule(Module);
	return Sound;
}
