#include "TaskDispatcher.h"

using namespace cliqCity::multicore;

TaskDispatcher::TaskDispatcher(Thread* threads, uint8_t threadCount, void* memory, size_t size) :
	mTaskGeneration(0),
	mAllocator(memory, reinterpret_cast<char*>(memory) + size, sizeof(Task)),
	mThreads(threads),
	mThreadCount(threadCount),
	mIsProcessingTasks(false),
	mMemory(memory)
{

}

TaskDispatcher::TaskDispatcher() : TaskDispatcher(nullptr, 0, nullptr, 0)
{

}

TaskDispatcher::~TaskDispatcher()
{
	mIsProcessingTasks = false;
	Synchronize();
	mThreads = nullptr;
}

void TaskDispatcher::Start()
{
	if (mIsProcessingTasks)
	{
		return;
	}

	mIsProcessingTasks = true;
	for (int i = 0; i < mThreadCount; i++)
	{
		mThreads[i] = Thread::thread(&TaskDispatcher::ProcessTasks, this);
	}
}

void TaskDispatcher::Pause()
{
	mIsProcessingTasks = false;
}

void TaskDispatcher::Synchronize()
{
	for (int i = 0; i < mThreadCount; i++)
	{
		if (mThreads[i].joinable())
		{
			mThreads[i].join();
		}
	}
}

bool TaskDispatcher::IsProcessing()
{
	return mIsProcessingTasks;
}

TaskID TaskDispatcher::AddTask(const TaskData& data, TaskKernel kernel)
{
	Task* task		= AllocateTask();
	task->mData		= data;
	task->mKernel	= kernel;

	TaskID taskID = GetTaskID(task);

	QueueTask(task);

	return taskID;
}

void TaskDispatcher::WaitForTask(const TaskID& taskID) const
{
	while (!IsTaskFinished(taskID))
	{
		std::this_thread::yield();
	}
}

bool TaskDispatcher::IsTaskFinished(const TaskID& taskID) const
{
	Task* task = GetTask(taskID);
	if (task->mGeneration != taskID.mGeneration)
	{
		return true;
	}
	else
	{
		// TO DO: Check dependent tasks.
	}

	return false;
}

inline Task* TaskDispatcher::WaitForAvailableTasks()
{
	UniqueLock lock(mQueueLock);
	while (mTaskQueue.empty())
	{
		mTaskSignal.wait(lock);
	}

	Task* task = mTaskQueue.front();
	mTaskQueue.pop();

	return task;
}

inline Task* TaskDispatcher::AllocateTask()
{
	Task* task = nullptr;
	{
		ScopedLock lock(mMemoryLock);
		task = reinterpret_cast<Task*>(mAllocator.Allocate());
	}

	task->mGeneration = ++mTaskGeneration;
	return task;
}

inline void TaskDispatcher::FreeTask(Task* task)
{
	task->mGeneration = ++mTaskGeneration;
	{
		ScopedLock lock(mMemoryLock);
		mAllocator.Free(task);
	}
}

inline void TaskDispatcher::QueueTask(Task* task)
{
	{
		UniqueLock lock(mQueueLock);
		mTaskQueue.push(task);
	}

	if (mIsProcessingTasks)
	{
		mTaskSignal.notify_all();
	}
}

inline void TaskDispatcher::ExecuteTask(Task* task)
{
	(task->mKernel)(task->mData);
}

inline void TaskDispatcher::ProcessTasks()
{
	while (mIsProcessingTasks)
	{
		Task* task = WaitForAvailableTasks();
		ExecuteTask(task);
		FreeTask(task);
	}
}

inline TaskID TaskDispatcher::GetTaskID(Task* task) const
{
	return TaskID(task - reinterpret_cast<Task*>(mMemory), task->mGeneration);
}

inline Task* TaskDispatcher::GetTask(const TaskID& taskID) const
{
	return reinterpret_cast<Task*>(mMemory) + taskID.mOffset;
}
