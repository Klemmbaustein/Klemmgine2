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
		* Allocates the default thread pool that can be accessed using ThreadPool::Main().
		* 
		* The default thread pool will have as many threads as logical processors on the CPU,
		* or 16, whichever is lower.
		* 
		* @see ThreadPool::Main()
		*/
		static void AllocateDefaultThreadPool();
		/**
		* @brief
		* Frees the default thread pool.
		* 
		* This will also wait for all active workers to finish their tasks.
		*/
		static void FreeDefaultThreadPool();

		/**
		* @brief
		* Creates a new thread pool
		* 
		* @param MaxJobs
		* The number of threads to allocate.
		* 
		* @param PoolName
		* The name of the thread pool that is displayed in debuggers and error messages.
		* 
		* @see ThreadPool::Main()
		*/
		ThreadPool(size_t MaxJobs, string PoolName);
		~ThreadPool();

		/**
		* @brief
		* Queues a job in this thread pool.
		* 
		* Jobs on the queue will be processed by different workers in the order they were added to
		* the queue.
		*/
		void AddJob(const ThreadFunction& Job);
		
		/**
		* @brief
		* Gets the main thread pool, which is allocated by the engine by default.
		* 
		* It usually makes more sense to use this thread pool instead of creating a new one.
		*/
		static ThreadPool* Main();

		/**
		* @brief
		* Returns the number of workers this thread pool owns.
		*/
		uint32 NumJobs() const;

		/**
		* @brief
		* Returns the name of this thread pool.
		*/
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