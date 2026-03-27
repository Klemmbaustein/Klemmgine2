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

	this->SoundFile.OnChanged = [this] {
		Component->Load(SoundFile.Value);
		Component->Play();
	};

	Component->Load(SoundFile.Value);
	Component->Play();

}

void engine::SoundObject::OnDestroyed()
{
}
