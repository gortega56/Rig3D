#pragma once
#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <atomic>
#include "Task.h"
#include "Memory\Memory\PoolAllocator.h"

namespace cliqCity
{
	namespace multicore
	{
		typedef cliqCity::memory::PoolAllocator TaskPool;
		typedef std::atomic<uint32_t>			AtomicCounter;
		typedef std::condition_variable			Signal;
		typedef std::mutex						Mutex;
		typedef std::unique_lock<Mutex>			UniqueLock;
		typedef std::lock_guard<Mutex>			ScopedLock;
		typedef std::thread						Thread;
		typedef std::queue<Task*>				Queue;
		
		class TaskDispatcher
		{
		public:
			TaskDispatcher(Thread* threads, uint8_t threadCount, void* memory, size_t size);
			TaskDispatcher();
			~TaskDispatcher();

			void Start();
			void Pause();
			void Synchronize();
			bool IsProcessing();

			TaskID  AddTask(const TaskData& data, TaskKernel kernel);

			void WaitForTask(const TaskID& taskID) const;
			bool IsTaskFinished(const TaskID& taskID) const;

		private:
			 AtomicCounter	mTaskGeneration;
			 Signal			mTaskSignal;
			 Mutex			mMemoryLock;
			 Mutex			mQueueLock;
			 Queue			mTaskQueue;
			 TaskPool		mAllocator;
			 void*			mMemory;
			 Thread*		mThreads;
			 uint8_t		mThreadCount;
			 bool			mIsProcessingTasks;

			 TaskID	GetTaskID(Task* task) const;
			 Task*	GetTask(const TaskID& taskID) const;

			 Task*	WaitForAvailableTasks();
			 Task*	AllocateTask();
			 void	FreeTask(Task* task);
			 void	QueueTask(Task* task);
			 void	ExecuteTask(Task* task);
			 void	ProcessTasks();
		};
	}
}