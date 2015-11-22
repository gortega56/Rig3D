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

	VClearRenderTargetViews();
	VClearRenderTextures();
}

ID3D11DepthStencilView* DX11RenderContext::GetDepthStencilView()
{
	return mDepthStencilView;
}

ID3D11RenderTargetView** DX11RenderContext::GetRenderTargetViews()
{
	return &mRenderTargetViews[0];
}

ID3D11Texture2D** DX11RenderContext::GetRenderTextures()
{
	return &mRenderTextures[0];
}

uint32_t  DX11RenderContext::GetRenderTargetViewCount() const
{
	return mRenderTargetViews.size();
}

uint32_t DX11RenderContext::GetRenderTextureCount() const
{
	return mRenderTextures.size();
}


void DX11RenderContext::SetDepthStencilView(ID3D11DepthStencilView* depthStencilView)
{
	mDepthStencilView = depthStencilView;
}

void DX11RenderContext::SetRenderTargetViews(std::vector<ID3D11RenderTargetView*>& renderTargetViews)
{
	VClearRenderTargetViews();

	mRenderTargetViews = renderTargetViews;
}

void DX11RenderContext::SetRenderTextures(std::vector<ID3D11Texture2D*>& renderTextures)
{
	VClearRenderTextures();

	mRenderTextures = renderTextures;
}


void DX11RenderContext::VClearRenderTargetViews()
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

void DX11RenderContext::VClearRenderTextures()
{
	for (ID3D11Texture2D* texture2D : mRenderTextures)
	{
		if (texture2D)
		{
			texture2D->Release();
		}
	}

	mRenderTextures.clear();
}