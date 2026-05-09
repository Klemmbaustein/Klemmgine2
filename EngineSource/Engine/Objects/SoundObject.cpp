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

	Component->Load(SoundFile.Value);
	Component->Play(Loop.Value, Is3D.Value);
}

void engine::SoundObject::OnDestroyed()
{
}
