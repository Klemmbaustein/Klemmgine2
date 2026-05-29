#include "Sound.h"
#include <Engine/File/Resource.h>
#include <AL/alext.h>
#include <AL/efx-presets.h>
#include <Core/ThreadPool.h>
#include <stdexcept>
#include "SoundInternal/SoundInternal.h"
#include "Sources/WavSoundFileSource.h"
#include "Sources/FlacSoundFileSource.h"
#include <Engine/Engine.h>

// TODO: fully implement

using namespace engine::subsystem;
using namespace engine::sound;
using namespace engine;
using graphics::BvhNode;
using graphics::BoundingBox;

static ALenum GetFormatFromSound(SoundData* Data)
{
	if (Data->IsStereo)
	{
		switch (Data->BitDepth)
		{
		case SoundBitDepth::Int8:
			return AL_FORMAT_STEREO8;
		case SoundBitDepth::Int16:
			return AL_FORMAT_STEREO16;
		case SoundBitDepth::Float32:
			return AL_FORMAT_STEREO_FLOAT32;
		default:
		case SoundBitDepth::Int32:
			throw std::runtime_error("not implemented");
		}
	}
	else
	{
		switch (Data->BitDepth)
		{
		case SoundBitDepth::Int8:
			return AL_FORMAT_MONO8;
		case SoundBitDepth::Int16:
			return AL_FORMAT_MONO16;
		case SoundBitDepth::Float32:
			return AL_FORMAT_MONO_FLOAT32;
		default:
		case SoundBitDepth::Int32:
			throw std::runtime_error("not implemented");
		}
	}
}

engine::sound::SoundDevice::SoundDevice(subsystem::Subsystem* System)
{
	this->DeviceData = new SoundDevice_Private();
	this->DeviceData->Device = alcOpenDevice(nullptr);

	System->Print(str::Format("Opened sound device: %s",
		alcGetString(this->DeviceData->Device, ALC_DEVICE_SPECIFIER)),
		Subsystem::LogType::Note);
	LoadExtensions();
}

engine::sound::SoundDevice::~SoundDevice()
{
	alcCloseDevice(this->DeviceData->Device);
	delete this->DeviceData;
}

void engine::sound::SoundDevice::LoadExtensions()
{
	if (alcIsExtensionPresent(this->DeviceData->Device, "ALC_EXT_EFX"))
	{
#define LOAD_PROC(type, name) this->DeviceData->name = reinterpret_cast<type>(alGetProcAddress(# name))

		LOAD_PROC(LPALGENFILTERS, alGenFilters);
		LOAD_PROC(LPALDELETEFILTERS, alDeleteFilters);
		LOAD_PROC(LPALISFILTER, alIsFilter);
		LOAD_PROC(LPALFILTERI, alFilteri);
		LOAD_PROC(LPALFILTERIV, alFilteriv);
		LOAD_PROC(LPALFILTERF, alFilterf);
		LOAD_PROC(LPALFILTERFV, alFilterfv);
		LOAD_PROC(LPALGETFILTERI, alGetFilteri);
		LOAD_PROC(LPALGETFILTERIV, alGetFilteriv);
		LOAD_PROC(LPALGETFILTERF, alGetFilterf);
		LOAD_PROC(LPALGETFILTERFV, alGetFilterfv);

		LOAD_PROC(LPALGENEFFECTS, alGenEffects);
		LOAD_PROC(LPALDELETEEFFECTS, alDeleteEffects);
		LOAD_PROC(LPALISEFFECT, alIsEffect);
		LOAD_PROC(LPALEFFECTI, alEffecti);
		LOAD_PROC(LPALEFFECTIV, alEffectiv);
		LOAD_PROC(LPALEFFECTF, alEffectf);
		LOAD_PROC(LPALEFFECTFV, alEffectfv);
		LOAD_PROC(LPALGETEFFECTI, alGetEffecti);
		LOAD_PROC(LPALGETEFFECTIV, alGetEffectiv);
		LOAD_PROC(LPALGETEFFECTF, alGetEffectf);
		LOAD_PROC(LPALGETEFFECTFV, alGetEffectfv);

		LOAD_PROC(LPALGENAUXILIARYEFFECTSLOTS, alGenAuxiliaryEffectSlots);
		LOAD_PROC(LPALDELETEAUXILIARYEFFECTSLOTS, alDeleteAuxiliaryEffectSlots);
		LOAD_PROC(LPALISAUXILIARYEFFECTSLOT, alIsAuxiliaryEffectSlot);
		LOAD_PROC(LPALAUXILIARYEFFECTSLOTI, alAuxiliaryEffectSloti);
		LOAD_PROC(LPALAUXILIARYEFFECTSLOTIV, alAuxiliaryEffectSlotiv);
		LOAD_PROC(LPALAUXILIARYEFFECTSLOTF, alAuxiliaryEffectSlotf);
		LOAD_PROC(LPALAUXILIARYEFFECTSLOTFV, alAuxiliaryEffectSlotfv);
		LOAD_PROC(LPALGETAUXILIARYEFFECTSLOTI, alGetAuxiliaryEffectSloti);
		LOAD_PROC(LPALGETAUXILIARYEFFECTSLOTIV, alGetAuxiliaryEffectSlotiv);
		LOAD_PROC(LPALGETAUXILIARYEFFECTSLOTF, alGetAuxiliaryEffectSlotf);
		LOAD_PROC(LPALGETAUXILIARYEFFECTSLOTFV, alGetAuxiliaryEffectSlotfv);
	}
}

engine::sound::SoundContext::SoundContext(SoundDevice* Device)
{
	SoundData = new SoundContext_Private();
	this->Device = Device;
	AddFileSource(new WavSoundFileSource());
	SoundData->Context = alcCreateContext(Device->DeviceData->Device, nullptr);

	MakeCurrent();

	ALuint Effect;
	Device->DeviceData->alGenEffects(1, &Effect);

	EFXEAXREVERBPROPERTIES* reverb = new EFXEAXREVERBPROPERTIES(EFX_REVERB_PRESET_WOODEN_LARGEROOM);

	Device->DeviceData->alEffecti(Effect, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_DENSITY, reverb->flDensity);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_DIFFUSION, reverb->flDiffusion);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_GAIN, reverb->flGain);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_GAINHF, reverb->flGainHF);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_GAINLF, reverb->flGainLF);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_DECAY_TIME, reverb->flDecayTime);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_DECAY_HFRATIO, reverb->flDecayHFRatio);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_DECAY_LFRATIO, reverb->flDecayLFRatio);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_REFLECTIONS_GAIN, reverb->flReflectionsGain);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_REFLECTIONS_DELAY, reverb->flReflectionsDelay);
	Device->DeviceData->alEffectfv(Effect, AL_EAXREVERB_REFLECTIONS_PAN, reverb->flReflectionsPan);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_LATE_REVERB_GAIN, reverb->flLateReverbGain);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_LATE_REVERB_DELAY, reverb->flLateReverbDelay);
	Device->DeviceData->alEffectfv(Effect, AL_EAXREVERB_LATE_REVERB_PAN, reverb->flLateReverbPan);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_ECHO_TIME, reverb->flEchoTime);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_ECHO_DEPTH, reverb->flEchoDepth);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_MODULATION_TIME, reverb->flModulationTime);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_MODULATION_DEPTH, reverb->flModulationDepth);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, reverb->flAirAbsorptionGainHF);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_HFREFERENCE, reverb->flHFReference);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_LFREFERENCE, reverb->flLFReference);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, reverb->flRoomRolloffFactor);
	Device->DeviceData->alEffecti(Effect, AL_EAXREVERB_DECAY_HFLIMIT, reverb->iDecayHFLimit);

	delete reverb;

	Device->DeviceData->alGenAuxiliaryEffectSlots(1, &this->SoundData->Effects);
	Device->DeviceData->alAuxiliaryEffectSloti(this->SoundData->Effects, AL_EFFECTSLOT_EFFECT, Effect);
	Device->DeviceData->alAuxiliaryEffectSlotf(this->SoundData->Effects, AL_EFFECTSLOT_GAIN, 0);

	auto Error = alGetError();

	if (Error != AL_NO_ERROR)
	{
		auto str = alGetString(Error);
		Log::Error(str);
	}
}

engine::sound::SoundContext::~SoundContext()
{
	std::lock_guard g{ ThreadData->SoundUpdateMutex };
	ThreadData->Stop = true;
	MakeCurrent();
	alcDestroyContext(this->SoundData->Context);
	delete this->SoundData;

	for (auto& i : this->Sources)
	{
		delete i;
	}
}

SoundBuffer* engine::sound::SoundContext::LoadSoundEffect(string Path)
{
	auto Found = Device->Buffers.find(Path);

	if (Found != Device->Buffers.end())
	{
		Found->second->References++;
		return Found->second;
	}

	IBinaryStream* File = resource::GetBinaryFile(Path);

	if (!File)
	{
		return nullptr;
	}

	// TODO: Sources store their supported extensions, all sources supporting the extension
	// are checked. (ParseSoundFile returns null when the file doesn't match it's format)
	auto Data = Sources[0]->ParseSoundFile(File);

	if (!Data)
	{
		return nullptr;
	}

	SoundBuffer* NewBuffer = new SoundBuffer();

	NewBuffer->ALBuffer;

	std::lock_guard g{ ThreadData->SoundUpdateMutex };
	MakeCurrent();
	alGenBuffers(1, &NewBuffer->ALBuffer);

	alBufferData(NewBuffer->ALBuffer, GetFormatFromSound(Data),
		Data->SoundBytes, Data->NumBytes, Data->SampleRate);
	Device->Buffers.insert({ Path, NewBuffer });

	delete Data;
	delete File;
	return NewBuffer;
}

SoundSource* engine::sound::SoundContext::CreateSoundSource(SoundBuffer* With)
{
	std::lock_guard g{ ThreadData->SoundUpdateMutex };
	MakeCurrent();

	ALuint Source;
	alGenSources(1, &Source);
	alSource3i(Source, AL_AUXILIARY_SEND_FILTER, (ALint)this->SoundData->Effects, 0, AL_FILTER_NULL);
	alSourcei(Source, AL_BUFFER, With->ALBuffer);

	auto err = alGetError();

	if (err != AL_NO_ERROR)
	{
		Log::Critical(alGetString(err));
	}

	return new SoundSource{
		.ALSource = Source
	};
}

void engine::sound::SoundContext::SetSourcePosition(SoundSource* Source, Vector3 NewPosition)
{
	std::lock_guard g{ ThreadData->SoundUpdateMutex };
	MakeCurrent();
	alSource3f(Source->ALSource, AL_POSITION, NewPosition.X, NewPosition.Y, NewPosition.Z);
}

void engine::sound::SoundContext::SetSourceVelocity(SoundSource* Source, Vector3 NewVelocity)
{
	std::lock_guard g{ ThreadData->SoundUpdateMutex };
	MakeCurrent();
	alSource3f(Source->ALSource, AL_VELOCITY, NewVelocity.X, NewVelocity.Y, NewVelocity.Z);
}

void engine::sound::SoundContext::PlaySource(SoundSource* Source, bool Loop, bool Is3D)
{
	std::lock_guard g{ ThreadData->SoundUpdateMutex };
	MakeCurrent();
	alSourcei(Source->ALSource, AL_LOOPING, Loop);
	alSourcePlay(Source->ALSource);

	if (Is3D)
	{
		alSourcei(Source->ALSource, AL_DISTANCE_MODEL, AL_INVERSE_DISTANCE_CLAMPED);
		float Factor = 100.0f / 200.0f;
		alSourcef(Source->ALSource, AL_REFERENCE_DISTANCE, 1.0f);
		alSourcef(Source->ALSource, AL_ROLLOFF_FACTOR, Factor);
	}
}

void engine::sound::SoundContext::StopSource(SoundSource* Source)
{
	std::lock_guard g{ ThreadData->SoundUpdateMutex };
	MakeCurrent();
	alSourceStop(Source->ALSource);
}

void engine::sound::SoundContext::Update(graphics::Camera* FromCamera, debug::DebugDraw* Debug)
{
	if (BoundsModified)
	{
		Debug_ShowReverbAreas(Debug);
		BoundsModified = false;
	}

	Vector3 Pos = FromCamera->GetPosition();
	alListenerf(AL_GAIN, 10);

	Vector3 Directions[2] = {
		FromCamera->GetForward(),
		FromCamera->GetUp(),
	};

	ThreadPool::Main()->AddJob([this, Pos, Directions] {
		std::lock_guard g{ ThreadData->SoundUpdateMutex };

		MakeCurrent();
		alListener3f(AL_POSITION, Pos.X, Pos.Y, Pos.Z);

		alListenerfv(AL_ORIENTATION, &Directions[0].X);

		UpdateReverb(Pos);
		UpdateReverbVolumeTree();
	});
}

void engine::sound::SoundContext::FreeSoundSource(SoundSource* Source)
{
	std::lock_guard g{ ThreadData->SoundUpdateMutex };
	MakeCurrent();
	alDeleteSources(1, &Source->ALSource);
	delete Source;
}

void engine::sound::SoundContext::FreeSoundEffect(SoundBuffer* Buffer)
{
	if (!Buffer)
	{
		return;
	}

	Buffer->References--;

	std::lock_guard g{ ThreadData->SoundUpdateMutex };
	if (Buffer->References == 0)
	{
		for (auto it = Device->Buffers.begin(); it != Device->Buffers.end(); it++)
		{
			if (it->second == Buffer)
			{
				Device->Buffers.erase(it);
				break;
			}
		}
		MakeCurrent();
		alDeleteBuffers(1, &Buffer->ALBuffer);
		delete Buffer;
	}
}

void engine::sound::SoundContext::AddFileSource(SoundFileSource* Source)
{
	this->Sources.push_back(Source);
}

SoundReverbVolume* engine::sound::SoundContext::AddReverbVolume(Transform At)
{
	auto& NewVolume = ReverbVolumes.emplace_back();

	NewVolume.AtTransform = At;
	NewVolume.Data.Volume = 1;
	NewVolume.UpdateTransform();
	MarkReverbDirty();
	return &NewVolume;
}

void engine::sound::SoundContext::RemoveReverbVolume(SoundReverbVolume* Target)
{
	for (auto it = ReverbVolumes.begin(); it != ReverbVolumes.end(); it++)
	{
		if (&*it == Target)
		{
			if (it->Box)
			{
				delete it->Box;
			}

			ReverbVolumes.erase(it);
			MarkReverbDirty();
			break;
		}
	}
}

void engine::sound::SoundContext::UpdateReverbVolumeTransform(SoundReverbVolume* Target, Transform NewTransform)
{
	if (Target->AtTransform == NewTransform)
	{
		return;
	}

	Target->AtTransform = NewTransform;
	Target->UpdateTransform();
	MarkReverbDirty();
}

void engine::sound::SoundContext::Debug_ShowReverbAreas(debug::DebugDraw* DrawInstance)
{
	for (auto& i : this->ReverbVolumes)
	{
		if (i.Box)
		{
			delete i.Box;
			i.Box = nullptr;
		}
	}

	if (Engine::IsPlaying ? ShowDebugBounds : ShowDebugBoundsInEditor)
	{
		for (auto& i : this->ReverbVolumes)
		{
			i.Box = new debug::DebugBox(i.AtTransform, Vector3(0, 1, 1));
			DrawInstance->AddShape(i.Box);
		}
	}
}

void engine::sound::SoundContext::SetShowDebugBounds(bool NewShowDebugBounds)
{
	if (NewShowDebugBounds != ShowDebugBounds)
	{
		ShowDebugBounds = NewShowDebugBounds;
		BoundsModified = true;
	}
}

void engine::sound::SoundContext::MakeCurrent() const
{
	alcMakeContextCurrent(SoundData->Context);
}

void engine::sound::SoundContext::UpdateReverbVolumeTree()
{
	if (!this->TreeModified)
	{
		return;
	}

	std::list<std::pair<SoundReverbVolume*, BoundingBox>> Nodes;
	ThreadData->CurrentReverbVolumes = this->ReverbVolumes;

	for (auto& i : ThreadData->CurrentReverbVolumes)
	{
		Nodes.push_back({ &i, BoundingBox(0, 1).Translate(i.AtTransform) });
	}
	ThreadPool::Main()->AddJob([this, Data = this->ThreadData, Nodes = std::move(Nodes)] {

		auto NewTree = std::make_shared<BvhNode<SoundReverbVolume*>>(Nodes);
		std::lock_guard g{ Data->TreeBuildMutex };

		if (Data->Stop)
		{
			return;
		}

		this->ReverbTree = NewTree;
	});
}

void engine::sound::SoundContext::UpdateReverb(Vector3 AtPosition)
{
	std::lock_guard g{ ThreadData->TreeBuildMutex };
	if (!ReverbTree)
	{
		return;
	}
	std::vector<SoundReverbVolume*> Volumes;
	ReverbTree->QueryPoint(AtPosition, Volumes);

	bool FoundReverb = false;

	for (auto& i : Volumes)
	{
		Vector3 VolumeDistance = i->AtInverse.ApplyTo(AtPosition).Abs();
		if (VolumeDistance.X <= 1.0f && VolumeDistance.Y <= 1.0f && VolumeDistance.Z <= 1.0f)
		{
			this->CurrentReverb = i->Data;
			Vector3 Position, Scale;
			Rotation3 Rotation;

			i->AtInverse.Decompose(Position, Rotation, Scale);

			Vector3 AbsoluteDistance = (VolumeDistance * Scale) - (Scale - 0.1f);

			float Distance = std::max(std::max(AbsoluteDistance.X, AbsoluteDistance.Y), AbsoluteDistance.Z) * (1.0f / 0.1f);

			CurrentReverbIntensity = 1.0f - std::max(Distance, 0.0f);
			FoundReverb = true;
		}
	}
	if (!FoundReverb)
	{
		this->CurrentReverb = ReverbData();
	}
	Device->DeviceData->alAuxiliaryEffectSlotf(this->SoundData->Effects, AL_EFFECTSLOT_GAIN, CurrentReverb.Volume * CurrentReverbIntensity);
}

void engine::sound::SoundContext::MarkReverbDirty()
{
	TreeModified = true;
	BoundsModified = true;
}
