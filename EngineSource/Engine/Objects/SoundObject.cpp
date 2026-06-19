#include "SoundObject.h"

engine::SoundObject::SoundObject()
{
}

engine::SoundObject::~SoundObject()
{
}

void engine::SoundObject::Begin()
{
	Component = new SoundComponent();
	Attach(Component);

	SoundFile.OnChanged = [this] {
		Component->Load(SoundFile.Value);
		Component->Play(Loop.Value, Is3D.Value);
	};

	Loop.OnChanged = [this] {
		Component->Play(Loop.Value, Is3D.Value);
	};

	Is3D.OnChanged = Loop.OnChanged;

	Range.OnChanged = [this] {
		Component->SetRange(Range.Value);
	};

	Volume.OnChanged = [this] {
		Component->SetVolume(Volume.Value);
	};

	Pitch.OnChanged = [this] {
		Component->SetPitch(Pitch.Value);
	};

	Component->Load(SoundFile.Value);
	Component->SetRange(Range.Value);
	Component->SetRange(Volume.Value);
	Component->SetRange(Pitch.Value);
	Component->Play(Loop.Value, Is3D.Value);
}

void engine::SoundObject::OnDestroyed()
{
}
