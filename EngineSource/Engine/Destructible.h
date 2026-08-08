#pragma once
#include <Core/Event.h>

namespace engine
{
	class Destructible
	{
	public:

		virtual ~Destructible();

		Event<> OnDestroyedEvent;
	};
}