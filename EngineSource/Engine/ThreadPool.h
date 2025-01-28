#pragma once
#include <Engine/Types.h>
#include <mutex>
#include <thread>
#include <functional>
#include <queue>
#include <condition_variable>

namespace engine
{
	class ThreadPool
	{
	public:
		using ThreadFunction = std::function<void()>;

		static void AllocateDefaultThreadPool();

		ThreadPool(size_t MaxJobs, string PoolName);
		~ThreadPool();
		void AddJob(const ThreadFunction& Job);
		
		static ThreadPool* Main();

		uint32 NumJobs() const;

		string GetName() const;

	private:

		void ThreadMain(size_t ThreadId);
		ThreadFunction FindFunction();

		static ThreadPool* MainPool;
		string Name;
		bool ShouldQuit = false;
		std::mutex JobQueueMutex;
		std::vector<std::thread> Threads;
		std::condition_variable ThreadCondition;
		std::queue<ThreadFunction> Jobs;
	};
}