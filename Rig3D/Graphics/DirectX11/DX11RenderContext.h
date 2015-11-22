#pragma once
#include "Rig3D/Graphics/Interface/IRenderContext.h"
#include <d3d11.h>
#include <vector>

namespace Rig3D
{
	class DX11RenderContext : IRenderContext
	{
	public:
		DX11RenderContext();
		~DX11RenderContext();

		ID3D11DepthStencilView*		GetDepthStencilView();
		ID3D11RenderTargetView**	GetRenderTargetViews();
		uint32_t GetRenderTargetViewCount() const;

		void SetDepthStencilView(ID3D11DepthStencilView* depthStencilView);
		void SetRenderTargetViews(std::vector<ID3D11RenderTargetView*>& renderTargetViews);
	
	private:
		ID3D11DepthStencilView*					mDepthStencilView;
		std::vector<ID3D11RenderTargetView*>	mRenderTargetViews;

		void ClearRenderTargetViews();
	};
}