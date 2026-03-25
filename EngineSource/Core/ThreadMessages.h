#pragma once
#include <functional>
#include <mutex>
#include <memory>

namespace engine::thread
{
	class ThreadMessageQueue
	{
	public:

		void Run(std::function<void()> Function);

		void Update();

	private:

		std::vector<std::function<void()>> Messages;
		std::mutex MessagesMutex;
	};

	using ThreadMessagesRef = std::shared_ptr<ThreadMessageQueue>;
}