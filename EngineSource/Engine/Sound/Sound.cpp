#include "Sound.h"
#include <Engine/File/Resource.h>
#include <alext.h>
#include "SoundInternal/SoundInternal.h"
#include "Sources/WavSoundFileSource.h"

// TODO: fully implement

using namespace engine::subsystem;
using namespace engine::sound;
using namespace engine;

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
}

engine::sound::SoundDevice::~SoundDevice()
{
	alcCloseDevice(this->DeviceData->Device);
	delete this->DeviceData;
}

engine::sound::SoundContext::SoundContext(SoundDevice* Device)
{
	SoundData = new SoundContext_Private();
	AddFileSource(new WavSoundFileSource());
	SoundData->Context = alcCreateContext(Device->DeviceData->Device, nullptr);
}

engine::sound::SoundContext::~SoundContext()
{
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
	auto Found = Buffers.find(Path);

	if (Found != Buffers.end())
	{
		Found->second->References++;
		return Found->second;
	}

	ReadOnlyBufferStream* File = resource::GetBinaryFile(Path);

	// TODO: Sources store their supported extensions, all sources supporting the extension
	// are checked. (ParseSoundFile returns null when the file doesn't match it's format)
	auto Data = Sources[0]->ParseSoundFile(File);

	SoundBuffer* NewBuffer = new SoundBuffer();

	NewBuffer->ALBuffer;

	MakeCurrent();
	alGenBuffers(1, &NewBuffer->ALBuffer);

	alBufferData(NewBuffer->ALBuffer, GetFormatFromSound(Data),
		Data->SoundBytes, Data->NumBytes, Data->SampleRate);

	delete Data;
	delete File;
	return NewBuffer;
}

SoundSource* engine::sound::SoundContext::CreateSoundSource(SoundBuffer* With)
{
	ALuint Source;

	MakeCurrent();
	alGenSources(1, &Source);

	alSourcei(Source, AL_DISTANCE_MODEL, AL_INVERSE_DISTANCE);
	alSourcei(Source, AL_BUFFER, With->ALBuffer);
	alSourcei(Source, AL_ROLLOFF_FACTOR, 2.0f);

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
	MakeCurrent();
	alSource3f(Source->ALSource, AL_POSITION, NewPosition.X, NewPosition.Y, NewPosition.Z);
}

void engine::sound::SoundContext::SetSourceVelocity(SoundSource* Source, Vector3 NewVelocity)
{
	MakeCurrent();
	alSource3f(Source->ALSource, AL_VELOCITY, NewVelocity.X, NewVelocity.Y, NewVelocity.Z);
}

void engine::sound::SoundContext::PlaySource(SoundSource* Source, bool Loop)
{
	MakeCurrent();
	alSourcei(Source->ALSource, AL_LOOPING, Loop);
	alSourcePlay(Source->ALSource);
}

void engine::sound::SoundContext::StopSource(SoundSource* Source)
{
	MakeCurrent();
	alSourceStop(Source->ALSource);
}

void engine::sound::SoundContext::Update(graphics::Camera* FromCamera)
{
	Vector3 Pos = FromCamera->GetPosition();

	MakeCurrent();
	alListener3f(AL_POSITION, Pos.X, Pos.Y, Pos.Z);

	Vector3 Directions[2] = {
		FromCamera->GetForward(),
		FromCamera->GetUp(),
	};

	alListenerfv(AL_ORIENTATION, &Directions[0].X);
}

void engine::sound::SoundContext::FreeSoundSource(SoundSource* Source)
{
	MakeCurrent();
	alDeleteSources(1, &Source->ALSource);
	delete Source;
}

void engine::sound::SoundContext::FreeSoundEffect(SoundBuffer* Buffer)
{
	Buffer->References--;

	if (Buffer->References == 0)
	{
		for (auto it = Buffers.begin(); it != Buffers.end(); it++)
		{
			if (it->second == Buffer)
			{
				Buffers.erase(it);
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

void engine::sound::SoundContext::MakeCurrent() const
{
	alcMakeContextCurrent(SoundData->Context);
}
