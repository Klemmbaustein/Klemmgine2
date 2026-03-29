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
		Component->Play(Loop.Value);
	};

	Loop.OnChanged = [this] {
		Component->Play(Loop.Value);
	};

	Component->Load(SoundFile.Value);
	Component->Play(Loop.Value);
}

void engine::SoundObject::OnDestroyed()
{
}
