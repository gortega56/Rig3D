#include "Engine.h"
#include "WMEventHandler.h"

#define WNDCLASSNAME L"Rig3DWindow"

using namespace Rig3D;

Engine::Engine() : mShouldQuit(false)
{
	mEventHandler = &WMEventHandler::SharedInstance();
}

Engine::~Engine()
{
}

int Engine::Initialize(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	

	WNDCLASSEX ex;

	ex.cbSize = sizeof(WNDCLASSEX);
	ex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	ex.lpfnWndProc = WinProc;
	ex.cbClsExtra = 0;
	ex.cbWndExtra = 0;
	ex.hInstance = hInstance;
	ex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	ex.hCursor = LoadCursor(NULL, IDC_ARROW);
	ex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	ex.lpszMenuName = NULL;
	ex.lpszClassName = WNDCLASSNAME;
	ex.hIconSm = NULL;

	RegisterClassEx(&ex);

	mEventHandler->RegisterObserver(WM_CLOSE, this);
	mEventHandler->RegisterObserver(WM_QUIT, this);
	mEventHandler->RegisterObserver(WM_DESTROY, this);

	// Create the window 

	mHwnd = CreateWindowEx(NULL,
		WNDCLASSNAME,
		L"Rig3D",
		WS_OVERLAPPEDWINDOW,
		0, 0,
		300, 300,
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(mHwnd, SW_SHOW);
	UpdateWindow(mHwnd);

	// The message loop 

	while (!mShouldQuit)
	{
		mEventHandler->Update();
	}

	return 0;
}

void Engine::HandleEvent(const IEvent& iEvent)
{
	const WMEvent& wmEvent = (const WMEvent&)iEvent;
	switch (wmEvent.msg)
	{
	case WM_QUIT:
		mShouldQuit = true;
		break;
	case WM_CLOSE:
		DestroyWindow(mHwnd);		
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}
}