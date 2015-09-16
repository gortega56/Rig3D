#pragma once
#include <chrono>

#pragma warning (disable: 4251)

#ifdef _WINDLL
#define RIG3D __declspec(dllexport)
#else
#define RIG3D __declspec(dllimport)
#endif

namespace Rig3D
{
	typedef std::chrono::high_resolution_clock::time_point				ClockTime;
	typedef std::chrono::duration<double, std::milli>					Milliseconds;

	class RIG3D Timer
	{
	public:
		static Timer& SharedInstance();
		double GetApplicationTime();
		double GetDeltaTime();
		void Reset();
		void Update(double* deltaTime);

	private:
		ClockTime mApplicationStartTime;
		ClockTime mSystemCurrentTime;
		double mDeltaTime;

		Timer();
		~Timer();

		Timer(Timer const&) = delete;
		void operator=(Timer const&) = delete;
	};
}

