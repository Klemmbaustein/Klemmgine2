#pragma once
#include <Core/Types.h>
#include <mutex>
#include <thread>
#include <functional>
#include <queue>
#include <condition_variable>

namespace engine
{
	/**
	* @brief
	* A thread pool class
	* 
	* Jobs can be added to the thread pool, and they will be
	* asynchronously processed by the threads of this pool.
	* 
	* @ingroup engine-core
	*/
	class ThreadPool
	{
	public:
		using ThreadFunction = std::function<void()>;

		/**
		* @brief
		* Allocates the default thread pool that can be accessed using ThreadPool::Main()
		*/
		static void AllocateDefaultThreadPool();
		static void FreeDefaultThreadPool();

		/**
		* @brief
		* Creates a new thread pool
		* 
		* @param MaxJobs
		* The number of threads to allocate.1
		*/
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