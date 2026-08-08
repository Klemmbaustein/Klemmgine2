#include "Destructible.h"

engine::Destructible::~Destructible()
{
	this->OnDestroyedEvent.Invoke();
}
