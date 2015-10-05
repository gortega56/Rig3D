#pragma once
#include "Rig3D\rig_defines.h"
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

		inline uint32_t GetIndexCount()		const { return mIndexCount; };
		inline uint32_t GetVertexStride()	const { return mVertexStride; };

	protected:
		uint32_t		mVertexStride;
		uint32_t		mIndexCount;
	};
}

