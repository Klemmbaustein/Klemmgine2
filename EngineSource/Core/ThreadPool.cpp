#include "ThreadPool.h"
#include <Core/Log.h>
#include <Core/Platform/Platform.h>
using namespace engine;

ThreadPool* ThreadPool::MainPool = nullptr;

void engine::ThreadPool::AllocateDefaultThreadPool()
{
	if (MainPool)
		FreeDefaultThreadPool();

	MainPool = new ThreadPool(std::min(std::thread::hardware_concurrency(), uint32(16)), "Klemmgine worker");
}

void engine::ThreadPool::FreeDefaultThreadPool()
{
	if (MainPool)
		delete MainPool;
	MainPool = nullptr;
}

engine::ThreadPool::ThreadPool(size_t MaxJobs, string PoolName)
{
	this->Name = PoolName;
	for (size_t i = 0; i < MaxJobs; i++)
	{
		Threads.emplace_back(&ThreadPool::ThreadMain, this, i);
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

uint32 engine::ThreadPool::NumJobs() const
{
	return uint32(Threads.size());
}

string engine::ThreadPool::GetName() const
{
	return Name;
}

ThreadPool::ThreadFunction engine::ThreadPool::FindFunction()
{
	std::unique_lock g{ JobQueueMutex };

	while (Jobs.empty() && !ShouldQuit)
		ThreadCondition.wait(g);

	if (ShouldQuit)
		return nullptr;

	ThreadFunction Found = Jobs.front();
	Jobs.pop();
	return Found;
}

void engine::ThreadPool::ThreadMain(size_t ThreadId)
{
	internal::platform::SetThreadName(str::Format("%s - %i", Name.c_str(), int(ThreadId)));
	bool ShouldQuitThread = false;
	while (true)
	{
		const ThreadFunction& Found = FindFunction();
		if (ShouldQuit)
		{
			break;
		}
		if (!Found)
			continue;
		Found();
	}
}
