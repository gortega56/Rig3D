// Resources: http://blog.molecular-matters.com/2012/04/12/building-a-load-balanced-task-scheduler-part-2-task-model-relationships/

#pragma once
#include <stdint.h>

namespace cliqCity
{
	namespace multicore
	{
		struct TaskID
		{
			uint32_t mOffset;
			uint32_t mGeneration;

			TaskID(uint32_t offset, uint32_t generation) : mOffset(offset), mGeneration(generation) {};
		};

		struct TaskData
		{
			void* mKernelData;
		};
		
		typedef void(*TaskKernel)(const TaskData&);
		
		class Task
		{
		public:
			char		mAlias[sizeof(void*)];
			uint32_t	mGeneration;
			TaskData	mData;
			TaskKernel	mKernel;

			Task() : mGeneration(0), mKernel(nullptr) {};
			~Task() {};
		};
	}
}



