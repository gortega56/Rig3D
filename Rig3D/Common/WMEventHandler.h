#pragma once
#include <Windows.h>
#include "EventHandler\IEventHandler.h"

class WMEvent;
class WMObserver;
class WMEventHandler;

LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

#pragma region Windows Message Handler

class WMEventHandler : public IEventHandler
{
public:
	static	WMEventHandler& SharedInstance();
	void	Update();
	void	NotifyObservers(const IEvent& iEvent) override;

private:
	MSG mMSG;

	WMEventHandler();
	~WMEventHandler();

	WMEventHandler(WMEventHandler const&) = delete;
	void operator=(WMEventHandler const&) = delete;
};

#pragma endregion

#pragma region WMEvent

class WMEvent : public IEvent
{
public:
	HWND	hwnd;
	UINT	msg;
	WPARAM	wparam;
	LPARAM	lparam;

	WMEvent(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) : hwnd(hwnd), msg(msg), wparam(wparam), lparam(lparam){};
	~WMEvent(){};
private:
	WMEvent();
};

#pragma endregion