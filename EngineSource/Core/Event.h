#pragma once
#include <map>
#include <functional>

namespace engine
{
	/**
	 * @brief
	 * A simple event type that calls many callbacks of it's listeners.
	 * @tparam ...Args
	 * The arguments of the event.
	 */
	template<typename... Args>
	struct Event
	{
		using Function = std::function<void(Args...)>;

		std::map<void*, Function> Callbacks;

		/**
		 * @brief
		 * Invokes the event, notifying all listeners.
		 * @param ...args
		 * The arguments to invoke the event with
		 */
		void Invoke(Args... args)
		{
			for (const auto& [_, c] : Callbacks)
			{
				c(args...);
			}
		}

		/**
		 * @brief
		 * Adds a new listener to the event.
		 * @param Listener
		 * A unique pointer identifying this listener.
		 * @param New
		 * The listener callback to add to the event.
		 */
		void Add(void* Listener, Function New)
		{
			Callbacks[Listener] = New;
		}

		/**
		 * @brief
		 * Removes a listener
		 * @param Listener
		 * A unique pointer identifying the listener that was given to the event with the Add function.
		 */
		void Remove(void* Listener)
		{
			Callbacks.erase(Listener);
		}
	};
}