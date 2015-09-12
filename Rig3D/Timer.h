#pragma once

#ifdef _WINDLL
#define RIG3D __declspec(dllexport)
#else
#define RIG3D __declspec(dllimport)
#endif

namespace Rig3D
{
	class Timer
	{
	public:
		Timer();
		~Timer();
	};
}

