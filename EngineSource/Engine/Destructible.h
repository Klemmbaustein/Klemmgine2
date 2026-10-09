#pragma once
#include <Core/Event.h>

namespace engine
{
	class Destructible
	{
	public:

		virtual ~Destructible()
		{
			this->OnDestroyedEvent.Invoke();
		}

		Event<> OnDestroyedEvent;
	};
}