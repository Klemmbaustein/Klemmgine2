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
#include <Engine/Stats.h>

// TODO: fully implement

using namespace engine::subsystem;
using namespace engine::sound;
using namespace engine;
using graphics::BvhNode;

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

	Device->DeviceData->alGenAuxiliaryEffectSlots(1, &this->SoundData->Effects);
	Device->DeviceData->alAuxiliaryEffectSlotf(this->SoundData->Effects, AL_EFFECTSLOT_GAIN, 0);
	alListenerf(AL_GAIN, 0.0f);

	auto Error = alGetError();

	if (Error != AL_NO_ERROR)
	{
		auto str = alGetString(Error);
		Log::Error(str);
	}
}

void engine::sound::SoundContext::Restart()
{
	StopAll();
	std::lock_guard g{ ThreadData->SoundUpdateMutex };
	alListenerf(AL_GAIN, 0.0f);
	Initialized = false;
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
	if (Source->Is3D)
	{
		MakeCurrent();
		alSource3f(Source->ALSource, AL_POSITION, NewPosition.X, NewPosition.Y, NewPosition.Z);
	}
}

void engine::sound::SoundContext::SetSourceVolume(SoundSource* Source, float Volume)
{
	std::lock_guard g{ ThreadData->SoundUpdateMutex };
	MakeCurrent();
	alSourcef(Source->ALSource, AL_GAIN, Volume);
}

void engine::sound::SoundContext::SetSourcePitch(SoundSource* Source, float Pitch)
{
	std::lock_guard g{ ThreadData->SoundUpdateMutex };
	MakeCurrent();
	alSourcef(Source->ALSource, AL_PITCH, Pitch);
}

void engine::sound::SoundContext::SetSourceRange(SoundSource* Source, float Falloff)
{
	std::lock_guard g{ ThreadData->SoundUpdateMutex };
	MakeCurrent();
	float Factor = 10.0f / Falloff;
	alSourcef(Source->ALSource, AL_ROLLOFF_FACTOR, Factor);
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
	Source->Is3D = Is3D;

	if (Is3D)
	{
		alSourcei(Source->ALSource, AL_DISTANCE_MODEL, AL_INVERSE_DISTANCE_CLAMPED);
		alSourcef(Source->ALSource, AL_REFERENCE_DISTANCE, 1.0f);
		alSourcei(Source->ALSource, AL_SOURCE_RELATIVE, 0);
	}
	else
	{
		alSourcei(Source->ALSource, AL_DISTANCE_MODEL, AL_NONE);
		alSourcei(Source->ALSource, AL_SOURCE_RELATIVE, 1);
		alSource3f(Source->ALSource, AL_POSITION, 0, 0, 0);
	}
}

void engine::sound::SoundContext::PlayBuffer(SoundEffectCache* Cache, float Volume, float Pitch, Vector3 Position, float Falloff)
{
	auto Source = CreateSoundSource(Cache->Buffer);
	Source->CacheRef = Cache;

	SetSourceVolume(Source, Volume);
	SetSourcePitch(Source, Pitch);

	if (Falloff != -1)
	{
		SetSourcePosition(Source, Position);
	}
	PlaySource(Source, false, Falloff != -1);

	std::lock_guard g{ ThreadData->SoundUpdateMutex };
	PlayingSounds.push_back(PlayedSound{
		.Source = Source,
		.Buffer = Cache->Buffer,
		});
}

void engine::sound::SoundContext::OnSoundStopped(PlayedSound& Sound)
{
	if (Sound.Source->CacheRef)
	{
		Sound.Source->CacheRef->UseCount--;
	}

	MakeCurrent();
	alDeleteSources(1, &Sound.Source->ALSource);
	delete Sound.Source;
}

void engine::sound::SoundContext::StopSource(SoundSource* Source)
{
	std::lock_guard g{ ThreadData->SoundUpdateMutex };
	MakeCurrent();
	alSourceStop(Source->ALSource);
}

void engine::sound::SoundContext::StopAll()
{
	for (auto& i : PlayingSounds)
	{
		OnSoundStopped(i);
	}
	PlayingSounds.clear();
}

void engine::sound::SoundContext::SetVolume(float NewVolume)
{
	std::lock_guard g{ ThreadData->SoundUpdateMutex };
	MakeCurrent();
	alListenerf(AL_GAIN, NewVolume);
	auto Error = alGetError();

	if (Error != AL_NO_ERROR)
	{
		auto str = alGetString(Error);
		Log::Error(str);
	}

	Initialized = true;
}

float engine::sound::SoundContext::GetVolume()
{
	std::lock_guard g{ ThreadData->SoundUpdateMutex };
	MakeCurrent();
	ALfloat Volume = 0;
	alGetListenerf(AL_GAIN, &Volume);
	return Volume;
}

void engine::sound::SoundContext::PlaySound(string Path, float Volume, float Pitch)
{
	PlaySoundAt(Path, Volume, Pitch, 0, -1);
}

void engine::sound::SoundContext::PlaySoundAt(string Path, float Volume, float Pitch, Vector3 Position, float Falloff)
{
	auto Found = CachedEffects.find(Path);

	if (Found != CachedEffects.end())
	{
		Found->second.LastUsed = stats::Time;
		Found->second.UseCount++;

		PlayBuffer(&Found->second, Volume, Pitch, Position, Falloff);

		return;
	}

	auto Buffer = LoadSoundEffect(Path);

	if (!Buffer)
	{
		Log::Warn(str::Format("Could not play sound %s because it's buffer could not be loaded", Path.c_str()));
		return;
	}

	while (CachedEffects.size() >= EffectCacheSize)
	{
		decltype(CachedEffects)::iterator FoundItem = CachedEffects.end();

		for (auto it = CachedEffects.begin(); it != CachedEffects.end(); it++)
		{
			if (it->second.UseCount > 0)
			{
				continue;
			}

			if (FoundItem != CachedEffects.end())
			{
				if (FoundItem->second.LastUsed > it->second.LastUsed)
				{
					FoundItem = it;
				}
			}
			else
			{
				FoundItem = it;
			}
		}

		if (FoundItem != CachedEffects.end())
		{
			FreeSoundEffect(FoundItem->second.Buffer);
			CachedEffects.erase(FoundItem);
		}
		else
		{
			break;
		}
	}

	auto c = SoundEffectCache{
		.LastUsed = stats::Time,
		.UseCount = 1,
		.Buffer = Buffer,
	};

	auto [Inserted, _] = CachedEffects.insert({ Path, c });

	PlayBuffer(&Inserted->second, Volume, Pitch, Position, Falloff);
}

void engine::sound::SoundContext::Update(graphics::Camera* FromCamera, debug::DebugDraw* Debug, bool Async)
{
	if (Debug && BoundsModified)
	{
		Debug_ShowReverbAreas(Debug);
		BoundsModified = false;
	}
	Vector3 Directions[2];
	Vector3 Pos;
	if (FromCamera)
	{
		Pos = FromCamera->GetPosition();
		//alListenerf(AL_GAIN, 1);

		Directions[0] = FromCamera->GetForward();
		Directions[1] = FromCamera->GetUp();
	}
	else
	{
		Directions[0] = Vector3(0, 0, -1);
		Directions[0] = Vector3(0, 1, 0);
	}

	ThreadPool::Main()->AddJob([this, Pos, Directions] {
		std::lock_guard g{ ThreadData->SoundUpdateMutex };

		MakeCurrent();
		alListener3f(AL_POSITION, Pos.X, Pos.Y, Pos.Z);

		alListenerfv(AL_ORIENTATION, &Directions[0].X);

		if (!Initialized)
		{
			alListenerf(AL_GAIN, 1.0f);
			Initialized = true;
		}

		UpdateReverb(Pos);
		UpdateReverbVolumeTree();
		for (auto it = PlayingSounds.begin(); it < PlayingSounds.end(); it++)
		{
			ALint SourceState = 0;
			alGetSourcei(it->Source->ALSource, AL_SOURCE_STATE, &SourceState);

			if (SourceState == AL_STOPPED)
			{
				OnSoundStopped(*it);
				PlayingSounds.erase(it);
				break;
			}
		}
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

SoundReverbVolume* engine::sound::SoundContext::AddReverbVolume(Transform At, ReverbData Data)
{
	auto& NewVolume = ReverbVolumes.emplace_back();

	NewVolume.AtTransform = At;
	NewVolume.Data = Data;
	NewVolume.UpdateTransform();
	NewVolume.VolumeData = std::make_shared<ReverbVolume_Private>();

	ALuint& Effect = NewVolume.VolumeData->ALEffectSlot;
	auto& Reverb = NewVolume.Data;

	MakeCurrent();
	Device->DeviceData->alGenEffects(1, &Effect);

	Device->DeviceData->alEffecti(Effect, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_DENSITY, Reverb.Density);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_DIFFUSION, Reverb.Diffusion);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_GAIN, Reverb.Gain);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_GAINHF, Reverb.HighFrequencyGain);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_GAINLF, Reverb.LowFrequencyGain);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_DECAY_TIME, Reverb.DecayTime);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_DECAY_HFRATIO, Reverb.DecayHighFrequencyRatio);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_DECAY_LFRATIO, Reverb.DecayLowFrequencyRatio);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_REFLECTIONS_GAIN, Reverb.ReflectionsGain);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_REFLECTIONS_DELAY, Reverb.ReflectionsDelay);
	Device->DeviceData->alEffectfv(Effect, AL_EAXREVERB_REFLECTIONS_PAN, &Reverb.ReflectionsPan[0]);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_LATE_REVERB_GAIN, Reverb.LateReverbGain);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_LATE_REVERB_DELAY, Reverb.LateReverbDelay);
	Device->DeviceData->alEffectfv(Effect, AL_EAXREVERB_LATE_REVERB_PAN, &Reverb.LateReverbPan[0]);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_ECHO_TIME, Reverb.EchoTime);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_ECHO_DEPTH, Reverb.EchoDepth);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_MODULATION_TIME, Reverb.ModulationTime);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_MODULATION_DEPTH, Reverb.ModulationDepth);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, Reverb.HighFrequencyAirAbsorptionGain);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_HFREFERENCE, Reverb.HighFrequencyReference);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_LFREFERENCE, Reverb.LowFrequencyReference);
	Device->DeviceData->alEffectf(Effect, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, Reverb.RoomRolloffFactor);
	Device->DeviceData->alEffecti(Effect, AL_EAXREVERB_DECAY_HFLIMIT, Reverb.DecayHighFrequencyLimit);

	auto Error = alGetError();

	if (Error != AL_NO_ERROR)
	{
		auto str = alGetString(Error);
		Log::Error(str);
	}

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
	if (alcGetCurrentContext() != SoundData->Context)
	{
		alcMakeContextCurrent(SoundData->Context);
	}
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
			CurrentReverb = i->Data;
			CurrentReverbData = i->VolumeData;
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
		CurrentReverb = ReverbData();
		CurrentReverbData = nullptr;
	}
	if (CurrentReverbData)
	{
		Device->DeviceData->alAuxiliaryEffectSloti(this->SoundData->Effects, AL_EFFECTSLOT_EFFECT, CurrentReverbData->ALEffectSlot);
	}
	Device->DeviceData->alAuxiliaryEffectSlotf(this->SoundData->Effects, AL_EFFECTSLOT_GAIN, CurrentReverb.Volume * CurrentReverbIntensity);
}

void engine::sound::SoundContext::MarkReverbDirty()
{
	TreeModified = true;
	BoundsModified = true;
}
