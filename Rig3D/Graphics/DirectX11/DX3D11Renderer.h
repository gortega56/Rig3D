#pragma once
#include "Rig3D\Graphics\Interface\IRenderer.h"
#include "Rig3D\Common\WMEventHandler.h"
#include "Rig3D\rig_graphics_api_conversions.h"
#include "Rig3D\Graphics\DirectX11\dxerr.h"
#include <assert.h>

#ifdef _WINDLL
#define RIG3D __declspec(dllexport)
#else
#define RIG3D __declspec(dllimport)
#endif

// Convenience macro for releasing a COM object
#define ReleaseMacro(x) { if(x){ x->Release(); x = 0; } }

// Macro for popping up a text box based
// on a failed HRESULT and then quitting (only in debug builds)
#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x)																\
				{															\
		HRESULT hr = (x);													\
		if(FAILED(hr))														\
						{													\
			DXTrace(__FILEW__, (DWORD)__LINE__, hr, L#x, true);				\
			PostQuitMessage(0);												\
						}													\
				}														
#endif
#else
#ifndef HR
#define HR(x) (x)
#endif
#endif

namespace Rig3D
{
	class RIG3D DX3D11Renderer : public IRenderer, public IObserver
	{
	public:
		static DX3D11Renderer& SharedInstance();

		int		VInitialize(HINSTANCE hInstance, HWND hwnd, int windowWidth, int windowHeight, const char* windowCaption) override;
		void	VUpdateScene(const double& milliseconds) override;
		void	VRenderScene() override;
		void	VOnResize() override;
		void	VShutdown() override;
		void	HandleEvent(const IEvent& iEvent) override;

		void	VDrawIndexed(GPU_PRIMITIVE_TYPE type, uint32_t startIndex, uint32_t count);


		int						InitializeD3D11();
		ID3D11Device*			GetDevice()				const;
		ID3D11DeviceContext*	GetDeviceContext()		const;
		IDXGISwapChain*			GetSwapChain()			const;
		ID3D11Texture2D*		GetDepthStencilBuffer() const;
		ID3D11RenderTargetView* const* GetRenderTargetView()	const;
		ID3D11DepthStencilView* GetDepthStencilView()	const;
		D3D11_VIEWPORT const&	GetViewport()			const;

	private:
		HINSTANCE				mHINSTANCE;
		HWND					mHWND;

		UINT					mMSAA4xQuality;
		ID3D11Device*			mDevice;
		ID3D11DeviceContext*	mDeviceContext;
		IDXGISwapChain*			mSwapChain;
		ID3D11Texture2D*		mDepthStencilBuffer;
		ID3D11RenderTargetView* mRenderTargetView;
		ID3D11DepthStencilView* mDepthStencilView;
		D3D11_VIEWPORT			mViewport;
		D3D_DRIVER_TYPE			mDriverType;
		D3D_FEATURE_LEVEL		mFeatureLevel;

		bool					mEnable4xMsaa;
		bool					mIsPaused;
		bool					mIsMaximized;
		bool					mIsMinimized;
		bool					mIsResizing;

		DX3D11Renderer();
		~DX3D11Renderer();

		DX3D11Renderer(DX3D11Renderer const&) = delete;
		void operator=(DX3D11Renderer const&) = delete;
	};
}

