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

		ThreadPool(size_t MaxJobs);
		~ThreadPool();
		void AddJob(const ThreadFunction& Job);
		
		static ThreadPool* Main();

	private:

		void ThreadMain();
		ThreadFunction FindFunction();

		static ThreadPool* MainPool;

		bool ShouldQuit = false;
		std::mutex JobQueueMutex;
		std::vector<std::thread> Threads;
		std::condition_variable ThreadCondition;
		std::queue<ThreadFunction> Jobs;
	};
}