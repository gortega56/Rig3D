#include "Rig3D\Graphics\DirectX11\DX3D11Renderer.h"
#include "Graphics\DirectX11\DX11Mesh.h"
#include "Rig3D\Engine.h"
#include "rig_defines.h"
#include "Rig3D\Graphics\Interface\IScene.h"
#include <WindowsX.h>
#include <sstream>

static wchar_t* WINDOW_CLASS_NAME = L"Rig3D DirectX11 Window";

using namespace Rig3D;

DX3D11Renderer& DX3D11Renderer::SharedInstance()
{
	static DX3D11Renderer sharedInstance;
	return sharedInstance;
}

DX3D11Renderer::DX3D11Renderer()
{
	mGraphicsAPI = GRAPHICS_API_DIRECTX11;
}


DX3D11Renderer::~DX3D11Renderer()
{

}

int DX3D11Renderer::VInitialize(HINSTANCE hInstance, HWND hwnd, Options options)
{
	mHINSTANCE				= hInstance;
	mHWND					= hwnd;
	mWindowWidth			= options.mWindowWidth;
	mWindowHeight			= options.mWindowHeight;
	mWindowCaption			= options.mWindowCaption;
	mFullScreen				= options.mFullScreen;
	mDriverType				= D3D_DRIVER_TYPE_HARDWARE;
	mEnable4xMsaa			= false;
	mMSAA4xQuality			= 0;
	mDevice					= 0;
	mDeviceContext			= 0;
	mDepthStencilBuffer		= 0;
	mDepthStencilView		= 0;
	mRenderTargetView		= 0;
	mSwapChain				= 0;

	ZeroMemory(&mViewport, sizeof(D3D11_VIEWPORT));

	if (InitializeD3D11() == RIG_ERROR) 
	{
		return RIG_ERROR;
	}

	WMEventHandler::SharedInstance().RegisterObserver(WM_SIZE, this);
	WMEventHandler::SharedInstance().RegisterObserver(WM_ENTERSIZEMOVE, this);
	WMEventHandler::SharedInstance().RegisterObserver(WM_EXITSIZEMOVE, this);

	return RIG_SUCCESS;
}

int DX3D11Renderer::InitializeD3D11()
{
	UINT createDeviceFlags = 0;

	// Do we want a debug device?
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Set up a swap chain description
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swapChainDesc.BufferDesc.Width = mWindowWidth;
	swapChainDesc.BufferDesc.Height = mWindowHeight;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = mHWND;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	// Enables Alt + Enter Windowed / Full Screen Toggle.
	if (mEnable4xMsaa)
	{
		// Set up 4x MSAA
		swapChainDesc.SampleDesc.Count = 4;
		swapChainDesc.SampleDesc.Quality = mMSAA4xQuality - 1;
	}
	else
	{
		// No MSAA
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
	}

	// Create the device and swap chain and determine the supported feature level
	mFeatureLevel = D3D_FEATURE_LEVEL_9_1; // Will be overwritten by next line
	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		0,
		mDriverType,
		0,
		createDeviceFlags,
		0,
		0,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&mSwapChain,
		&mDevice,
		&mFeatureLevel,
		&mDeviceContext);

	// Handle any device creation or DirectX version errors
	if (FAILED(hr))
	{
		MessageBox(0, L"D3D11CreateDevice Failed", 0, 0);
		return 1;
	}

	// Check for 4X MSAA quality support
	HR(mDevice->CheckMultisampleQualityLevels(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		4,
		&mMSAA4xQuality));
	assert(mMSAA4xQuality > 0); // Potential problem if quality is 0

	HR(mSwapChain->SetFullscreenState(mFullScreen, NULL));
	// The remaining steps also need to happen each time the window
	// is resized, so just run the OnResize method
	VOnResize();

	return 0;
}

void DX3D11Renderer::VUpdateScene(const double& milliseconds)
{

}

void DX3D11Renderer::VRenderScene()
{

}

void DX3D11Renderer::VSetMeshVertexBufferData(IMesh* mesh, void* vertices, const size_t& size, const size_t& stride, const GPUMemoryUsage& usage)
{
	DX11Mesh* dxMesh = (DX11Mesh*)mesh;
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = GD3D11_Usage_Map(usage);
	vbd.ByteWidth = size;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;	// Not used for Vertex Input

	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;
	mDevice->CreateBuffer(&vbd, &vertexData, &dxMesh->mVertexBuffer);

	dxMesh->mVertexStride = stride;
}

void DX3D11Renderer::VSetMeshIndexBufferData(IMesh* mesh, uint16_t* indices, const uint32_t& count, const GPUMemoryUsage& usage)
{
	DX11Mesh* dxMesh = (DX11Mesh*)mesh;
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = GD3D11_Usage_Map(usage);
	ibd.ByteWidth = sizeof(uint16_t) * count;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	mDevice->CreateBuffer(&ibd, &indexData, &dxMesh->mIndexBuffer);

	dxMesh->mIndexCount = count;
}

void DX3D11Renderer::VBindMesh(IMesh* mesh)
{
	DX11Mesh* dxMesh = (DX11Mesh*)mesh;
	uint32_t offset = 0;
	mDeviceContext->IASetVertexBuffers(0, 1, &dxMesh->mVertexBuffer, &dxMesh->mVertexStride, &offset);
	mDeviceContext->IASetIndexBuffer(dxMesh->mIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
}

void DX3D11Renderer::VSetPrimitiveType(GPUPrimitiveType type)
{
	mDeviceContext->IASetPrimitiveTopology(GD3D11_Primitive_Type_Map(type));
}

void DX3D11Renderer::VDrawIndexed(GPUPrimitiveType type, uint32_t startIndex, uint32_t count)
{
	mDeviceContext->IASetPrimitiveTopology(GD3D11_Primitive_Type_Map(type));
	mDeviceContext->DrawIndexed(
		count,
		startIndex,
		0);
}

void DX3D11Renderer::VDrawIndexed(uint32_t startIndex, uint32_t count)
{
	mDeviceContext->DrawIndexed(
		count,
		startIndex,
		0);
}

void DX3D11Renderer::VOnResize()
{
	// Release the views, since we'll be destroying
	// the corresponding buffers.
	ReleaseMacro(mRenderTargetView);
	ReleaseMacro(mDepthStencilView);
	ReleaseMacro(mDepthStencilBuffer);

	// Resize the swap chain to match the window and
	// recreate the render target view
	HR(mSwapChain->ResizeBuffers(
		1,
		mWindowWidth,
		mWindowHeight,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		0));
	ID3D11Texture2D* backBuffer;
	HR(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
	HR(mDevice->CreateRenderTargetView(backBuffer, 0, &mRenderTargetView));
	ReleaseMacro(backBuffer);

	// Set up the description of the texture to use for the
	// depth stencil buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = mWindowWidth;
	depthStencilDesc.Height = mWindowHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;
	if (mEnable4xMsaa)
	{
		// Turn on 4x MultiSample Anti Aliasing
		// This must match swap chain MSAA values
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = mMSAA4xQuality - 1;
	}
	else
	{
		// No MSAA
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	// Create the depth/stencil buffer and corresponding view
	HR(mDevice->CreateTexture2D(&depthStencilDesc, 0, &mDepthStencilBuffer));
	HR(mDevice->CreateDepthStencilView(mDepthStencilBuffer, 0, &mDepthStencilView));

	// Bind these views to the pipeline, so rendering actually
	// uses the underlying textures
	mDeviceContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);

	// Update the viewport and set it on the device
	mViewport.TopLeftX = 0;
	mViewport.TopLeftY = 0;
	mViewport.Width = (float)mWindowWidth;
	mViewport.Height = (float)mWindowHeight;
	mViewport.MinDepth = 0.0f;
	mViewport.MaxDepth = 1.0f;
	mDeviceContext->RSSetViewports(1, &mViewport);

	if (mDelegate) {
		mDelegate->VOnResize();
	}
}

void DX3D11Renderer::VSwapBuffers()
{
	mSwapChain->Present(0, 0);
}

void DX3D11Renderer::HandleEvent(const IEvent& iEvent)
{
	const WMEvent& wmEvent = (const WMEvent&)iEvent;
	switch (wmEvent.msg)
	{
	case WM_SIZE:
		// Save the new client area dimensions.
		mWindowWidth = LOWORD(wmEvent.lparam);
		mWindowHeight = HIWORD(wmEvent.lparam);
		if (mDevice)
		{
			if (wmEvent.wparam == SIZE_MINIMIZED)
			{
				mIsPaused = true;
				mIsMinimized = true;
				mIsMaximized = false;
			}
			else if (wmEvent.wparam == SIZE_MAXIMIZED)
			{
				mIsPaused = false;
				mIsMinimized = false;
				mIsMaximized = true;
				VOnResize();
			}
			else if (wmEvent.wparam == SIZE_RESTORED)
			{
				// Restoring from minimized state?
				if (mIsMinimized)
				{
					mIsPaused = false;
					mIsMinimized = false;
					VOnResize();
				}

				// Restoring from maximized state?
				else if (mIsMaximized)
				{
					mIsPaused = false;
					mIsMaximized = false;
					VOnResize();
				}
				else if (mIsResizing)
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					VOnResize();
				}
			}
		}
		break;
		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		mIsPaused = true;
		mIsResizing = true;
		//timer.Stop();
		break;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		mIsPaused = false;
		mIsResizing = false;
		//timer.Start();
		VOnResize();
		break;
	default:
		break;
	}
}

void DX3D11Renderer::VShutdown()
{
	ReleaseMacro(mDevice);
	ReleaseMacro(mDeviceContext);
	ReleaseMacro(mDepthStencilBuffer);
	ReleaseMacro(mDepthStencilView);
	ReleaseMacro(mRenderTargetView);
	ReleaseMacro(mSwapChain);
}

ID3D11Device* DX3D11Renderer::GetDevice() const
{
	return mDevice;
}

ID3D11DeviceContext* DX3D11Renderer::GetDeviceContext() const
{
	return mDeviceContext;
}

IDXGISwapChain*	DX3D11Renderer::GetSwapChain()	const
{
	return mSwapChain;
}

ID3D11Texture2D* DX3D11Renderer::GetDepthStencilBuffer() const
{
	return mDepthStencilBuffer;
}

ID3D11RenderTargetView* const* DX3D11Renderer::GetRenderTargetView() const
{
	return &mRenderTargetView;
}

ID3D11DepthStencilView* DX3D11Renderer::GetDepthStencilView()	const
{
	return mDepthStencilView;
}

D3D11_VIEWPORT const& DX3D11Renderer::GetViewport() const
{
	return mViewport;
}
