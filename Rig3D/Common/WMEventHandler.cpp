#include "WMEventHandler.h"
#include <Windows.h>

namespace
{
	WMEventHandler* gWMEventHandler = 0;
}

LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	 gWMEventHandler->NotifyObservers(WMEvent(hwnd, msg, wparam, lparam));
	 return DefWindowProc(hwnd, msg, wparam, lparam);
}

WMEventHandler& WMEventHandler::SharedInstance()
{
	static WMEventHandler sharedInstance;
	return sharedInstance;
}

WMEventHandler::WMEventHandler()
{
	gWMEventHandler = this;
}

WMEventHandler::~WMEventHandler()
{
}

void WMEventHandler::NotifyObservers(const IEvent& iEvent)
{
	const WMEvent& wmEvent = (const WMEvent&)iEvent;
	ObserverMap::iterator mapEntry = mObservers.find(wmEvent.msg);
	if (mapEntry != mObservers.end())
	{
		for (ObserverSet::const_iterator iter = mapEntry->second.begin(); iter != mapEntry->second.end(); iter++)
		{
			(*iter)->HandleEvent(wmEvent);
		}
	}
}

void WMEventHandler::Update()
{
	if (PeekMessage(&mMSG, NULL, NULL, NULL, PM_REMOVE))
	{
		TranslateMessage(&mMSG);
		DispatchMessage(&mMSG);

		// WM_QUIT will never be received via WindowsProc function.
		// Check deliberately here.
		if (mMSG.message == WM_QUIT) {
			gWMEventHandler->NotifyObservers(WMEvent(0, mMSG.message, 0, 0));
		}
	}
}