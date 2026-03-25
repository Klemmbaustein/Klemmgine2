#include "ThreadMessages.h"
#include <mutex>
#include <Core/Error/EngineError.h>

void engine::thread::ThreadMessageQueue::Run(std::function<void()> Function)
{
	std::lock_guard g{ MessagesMutex };
	Messages.push_back(Function);
}

void engine::thread::ThreadMessageQueue::Update()
{
	MessagesMutex.lock();
	std::vector Functions = Messages;
	Messages.clear();
	MessagesMutex.unlock();

	for (auto& i : Functions)
	{
		i();
	}
}
