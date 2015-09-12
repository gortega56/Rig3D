#pragma once
#include <Windows.h>
#include "WMEventHandler.h"

#ifdef _WINDLL
#define RIG3D __declspec(dllexport)
#else
#define RIG3D __declspec(dllimport)
#endif

namespace Rig3D
{
	class RIG3D Engine : public IObserver
	{
	public:
		WMEventHandler* mEventHandler;

		Engine();
		~Engine();

		int Initialize(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd);
		int Run();
		void HandleEvent(const IEvent& wmEvent) override;

	protected:
		HDC mHdc;
		HWND mHwnd;
	
	private:
		bool mShouldQuit;
	};
}



