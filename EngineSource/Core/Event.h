#pragma once
#include <map>
#include <functional>

namespace engine
{
	template<typename... Args>
	struct Event
	{
		using Function = std::function<void(Args...)>;

		std::map<void*, Function> Callbacks;

		void Invoke(Args... args)
		{
			for (const auto& [_, c] : Callbacks)
			{
				c(args...);
			}
		}

		void Add(void* Listener, Function New)
		{
			Callbacks[Listener] = New;
		}

		void Remove(void* Listener)
		{
			Callbacks.erase(Listener);
		}
	};
}