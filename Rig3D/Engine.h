#pragma once
#include <Windows.h>
#include "WMEventHandler.h"
#include "Timer.h"


#ifdef _WINDLL
#define RIG3D __declspec(dllexport)
#else
#define RIG3D __declspec(dllimport)
#endif

namespace Rig3D
{
	class IRenderer;

	class RIG3D Engine : public virtual IObserver
	{
	public:
		Engine();
		~Engine();

		int		Initialize(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmdEngine, int windowWidth, int windowHeight, const char* windowCaption);
		int		Run();
		void	HandleEvent(const IEvent& iEvent) override;

	protected:
		HDC mHDC;
		HWND mHWND;
	
	private:
		WMEventHandler* mEventHandler;
		Timer*			mTimer;
		IRenderer*		mRenderer;
		bool			mShouldQuit;

		int InitializeMainWindow(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd, int windowWidth, int windowHeight, const char* windowCaption);
	};
}



