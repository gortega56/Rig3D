#pragma once
#include <Windows.h>
#include "Rig3D\Common\WMEventHandler.h"
#include "Rig3D\Common\Timer.h"
#include "Rig3D\Common\Input.h"
#include "Rig3D\rig_defines.h"
#include "Rig3D\Options.h"
#include "Rig3D\Graphics\Interface\IRenderer.h"
#include "Rig3D\Graphics\DirectX11\DX3D11Renderer.h"
#include "Rig3D\TSingleton.h"

#ifdef _WINDLL
#define RIG3D __declspec(dllexport)
#else
#define RIG3D __declspec(dllimport)
#endif

static const float DEFAULT_WINDOW_SIZE = 800.0f;
static const wchar_t* WND_CLASS_NAME = L"Rig3D Window Class";

namespace Rig3D
{
	//class IRenderer;
	//class IScene;

	//class RIG3D Engine : public virtual IObserver
	//{
	//public:
	//	Engine();
	//	~Engine();

	//	int		Initialize(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmdEngine, Options options);
	//	void	BeginScene();
	//	void	EndScene();
	//	int		Shutdown();
	//	void	HandleEvent(const IEvent& iEvent) override;

	//	void	RunScene(IScene* iScene);

	//	IRenderer* GetRenderer() const;

	//protected:
	//	HDC mHDC;
	//	HWND mHWND;
	//
	//private:
	//	WMEventHandler* mEventHandler;
	//	Timer*			mTimer;
	//	Input*			mInput;
	//	IRenderer*		mRenderer;
	//	bool			mShouldQuit;

	//	int InitializeMainWindow(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd, Options options);
	//};

	class IScene;

	class RIG3D Engine : public virtual IObserver
	{
	public:
		Engine() : mShouldQuit(false)
		{

		}
		~Engine()
		{
			
		}

		int		Initialize(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd, Options options)
		{
			// Event Handler needs to be initialized before creating window.
			mEventHandler = &WMEventHandler::SharedInstance();
			mEventHandler->RegisterObserver(WM_CLOSE, this);
			mEventHandler->RegisterObserver(WM_QUIT, this);
			mEventHandler->RegisterObserver(WM_DESTROY, this);

			if (InitializeMainWindow(hInstance, prevInstance, cmdLine, showCmd, options) == RIG_ERROR)
			{
				return RIG_ERROR;
			}

			mRenderer = &TSingleton<IRenderer<DX3D11Renderer>>::SharedInstance();


			//mRenderer = (options.mGraphicsAPI == GRAPHICS_API_DIRECTX11) ? &DX3D11Renderer::SharedInstance() : NULL; // TO DO: OpenGL Renderer
			if (mRenderer->VInitialize(hInstance, mHWND, options) == RIG_ERROR)
			{
				return RIG_ERROR;
			}

			mInput = &Input::SharedInstance();
			if (mInput->Initialize() == RIG_ERROR)
			{
				return RIG_ERROR;
			}

			mTimer = &Timer::SharedInstance();

			return 0;
		}

		void	BeginScene()
		{
			// The message loop
			double deltaTime = 0.0;
			mTimer->Reset();
			while (!mShouldQuit)
			{
				mTimer->Update(&deltaTime);
				mEventHandler->Update();
				mRenderer->VUpdateScene(deltaTime);
				mRenderer->VRenderScene();
			}
		}

		void	EndScene();
		int		Shutdown()
		{
			mRenderer->VShutdown();
			return RIG_SUCCESS;
		}
		void	HandleEvent(const IEvent& iEvent) override
		{
			const WMEvent& wmEvent = static_cast<const WMEvent&>(iEvent);
			switch (wmEvent.msg)
			{
			case WM_QUIT:
				mShouldQuit = true;
				break;
			case WM_CLOSE:
				DestroyWindow(mHWND);
				break;
			case WM_DESTROY:
				PostQuitMessage(0);
				break;
			default:
				break;
			}
		}

		void	RunScene(IScene* iScene)
		{
			iScene->VInitialize();

			// The message loop
			double deltaTime = 0.0;
			mTimer->Reset();
			while (!mShouldQuit)
			{
				mTimer->Update(&deltaTime);
				mEventHandler->Update();
				iScene->VUpdate(deltaTime);
				iScene->VRender();

				mInput->Flush();
			}

			iScene->VShutdown();
			Shutdown();
		}

		IRenderer<DX3D11Renderer>* GetRenderer() const
		{
			return mRenderer;

		}

	protected:
		HDC mHDC;
		HWND mHWND;

	private:
		WMEventHandler* mEventHandler;
		Timer*			mTimer;
		Input*			mInput;
		IRenderer<DX3D11Renderer>*		mRenderer;
		bool			mShouldQuit;

		int InitializeMainWindow(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd, Options options)
		{
			WNDCLASSEX ex;
			ex.cbSize = sizeof(WNDCLASSEX);
			ex.style = CS_HREDRAW | CS_VREDRAW;
			ex.lpfnWndProc = WinProc;
			ex.cbClsExtra = 0;
			ex.cbWndExtra = 0;
			ex.hInstance = hInstance;
			ex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
			ex.hCursor = LoadCursor(NULL, IDC_ARROW);
			ex.lpszMenuName = NULL;
			ex.lpszClassName = WND_CLASS_NAME;
			ex.hIconSm = NULL;

			if (options.mFullScreen == false) {
				ex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
			}

			if (!RegisterClassEx(&ex)) {
				MessageBox(0, L"RegisterClass Failed.", 0, 0);
				return RIG_ERROR;
			}

			// Compute window rectangle dimensions based on requested client area dimensions.
			RECT R = { 0, 0, options.mWindowWidth, options.mWindowHeight };
			AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
			int width = R.right - R.left;
			int height = R.bottom - R.top;

			LPCWSTR wideWindowCaption;
			CSTR2WSTR(options.mWindowCaption, wideWindowCaption);

			// Create the window 

			mHWND = CreateWindowEx(NULL,
				WND_CLASS_NAME,
				wideWindowCaption,
				WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT, CW_USEDEFAULT,
				width, height,
				NULL,
				NULL,
				hInstance,
				NULL);

			ShowWindow(mHWND, SW_SHOW);
			UpdateWindow(mHWND);

			return RIG_SUCCESS;
		}
	};
}



