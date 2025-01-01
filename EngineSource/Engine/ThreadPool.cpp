#include "ThreadPool.h"
#include <Engine/Log.h>
using namespace engine;

ThreadPool* ThreadPool::MainPool = nullptr;

void engine::ThreadPool::AllocateDefaultThreadPool()
{
	MainPool = new ThreadPool(std::min(std::thread::hardware_concurrency() - 1, uint32(8)));
}

engine::ThreadPool::ThreadPool(size_t MaxJobs)
{
	for (size_t i = 0; i < MaxJobs; i++)
	{
		Threads.emplace_back(&ThreadPool::ThreadMain, this);
	}
}

engine::ThreadPool::~ThreadPool()
{
	{
		std::unique_lock<std::mutex> g{ JobQueueMutex };
		ShouldQuit = true;
	}
	ThreadCondition.notify_all();
	for (std::thread& Thread : Threads)
	{
		Thread.join();
	}
}

void engine::ThreadPool::AddJob(const ThreadFunction& Job)
{
	{
		std::unique_lock g{ JobQueueMutex };
		Jobs.push(Job);
	}
	ThreadCondition.notify_one();
}

ThreadPool* engine::ThreadPool::Main()
{
	return MainPool;
}

ThreadPool::ThreadFunction engine::ThreadPool::FindFunction()
{
	std::unique_lock g{ JobQueueMutex };
	ThreadCondition.wait(g);

	if (ShouldQuit)
		return nullptr;

	ThreadFunction Found = Jobs.front();
	Jobs.pop();
	return Found;
}

void engine::ThreadPool::ThreadMain()
{
	while (true)
	{
		const ThreadFunction& Found = FindFunction();
		if (!Found)
		{
			break;
		}
		Found();
	}
}
