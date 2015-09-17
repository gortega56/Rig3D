#pragma once
#include "rig_defines.h"
#include <stdint.h>

#ifdef _WINDLL
#define RIG3D __declspec(dllexport)
#else
#define RIG3D __declspec(dllimport)
#endif

namespace Rig3D
{
	class RIG3D IMesh
	{
	public:
		IMesh();
		virtual ~IMesh();

		virtual void VSetVertexBuffer(void* vertices, const size_t& size, const size_t& stride, const GPU_MEMORY_USAGE& usage) = 0;
		virtual void VSetIndexBuffer(uint16_t* indices, const uint32_t& count, const GPU_MEMORY_USAGE& usage) = 0;
		virtual void VBindVertexBuffer() = 0;
		virtual void VBindIndexBuffer() = 0;
	
	protected:
		uint32_t		mVertexStride;
		uint32_t		mIndexCount;
	};
}

