#include "DX11RenderContext.h"

using namespace Rig3D;

DX11RenderContext::DX11RenderContext() : mDepthStencilView(nullptr)
{
	
}


DX11RenderContext::~DX11RenderContext()
{
	if (mDepthStencilView)
	{
		mDepthStencilView->Release();
	}

	ClearRenderTargetViews();
}

ID3D11DepthStencilView* DX11RenderContext::GetDepthStencilView()
{
	return mDepthStencilView;
}

ID3D11RenderTargetView** DX11RenderContext::GetRenderTargetViews()
{
	return &mRenderTargetViews[0];
}

uint32_t  DX11RenderContext::GetRenderTargetViewCount() const
{
	return mRenderTargetViews.size();
}

void DX11RenderContext::SetDepthStencilView(ID3D11DepthStencilView* depthStencilView)
{
	mDepthStencilView = depthStencilView;
}

void DX11RenderContext::SetRenderTargetViews(std::vector<ID3D11RenderTargetView*>& renderTargetViews)
{
	ClearRenderTargetViews();

	mRenderTargetViews = renderTargetViews;
}

void DX11RenderContext::ClearRenderTargetViews()
{
	for (ID3D11RenderTargetView* rtv : mRenderTargetViews)
	{
		if (rtv)
		{
			rtv->Release();
		}
	}

	mRenderTargetViews.clear();
}