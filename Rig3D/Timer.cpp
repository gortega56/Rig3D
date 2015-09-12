#include "Timer.h"
//#include <stdio.h>
//#include <Windows.h>

using namespace Rig3D;

Timer& Timer::SharedInstance()
{
	static Timer sharedTimer;
	return sharedTimer;
}

Timer::Timer()
{
	
}

Timer::~Timer()
{
}


void Timer::GetApplicationTime(double* applicationTime)
{
	*applicationTime = Milliseconds(mSystemCurrentTime - mApplicationStartTime).count();
}

void Timer::GetDeltaTime(double* deltaTime)
{
	*deltaTime = mDeltaTime;
}

void Timer::Reset()
{
	mApplicationStartTime = mSystemCurrentTime = std::chrono::high_resolution_clock::now();
	mDeltaTime = 0.0;
}

void Timer::Update()
{
	ClockTime systemTime = std::chrono::high_resolution_clock::now();
	mDeltaTime = std::fmax((Milliseconds(systemTime - mSystemCurrentTime).count() + DBL_EPSILON), 0.0); //Force Non negative
	mSystemCurrentTime = systemTime;

	//char str[256];
	//sprintf_s(str, "Milliseconds %f\n", mDeltaTime);

	//wchar_t wtext[256];
	//size_t num = 0;
	//mbstowcs_s(&num, wtext, str, strlen(str) + 1);
	//LPWSTR ptr = wtext;

	//OutputDebugString(ptr);
}