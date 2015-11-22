#pragma once
#include "Rig3D/Graphics/Interface/IRenderContext.h"
#include <d3d11.h>
#include <vector>

namespace Rig3D
{
	class DX11RenderContext : public IRenderContext
	{
	public:
		DX11RenderContext();
		~DX11RenderContext();

		ID3D11DepthStencilView*		GetDepthStencilView();
		ID3D11RenderTargetView**	GetRenderTargetViews();
		ID3D11Texture2D**			GetRenderTextures();
		uint32_t GetRenderTargetViewCount() const;
		uint32_t GetRenderTextureCount() const;

		void SetDepthStencilView(ID3D11DepthStencilView* depthStencilView);
		void SetRenderTargetViews(std::vector<ID3D11RenderTargetView*>& renderTargetViews);
		void SetRenderTextures(std::vector<ID3D11Texture2D*>& renderTextures);
	
		void VClearRenderTargetViews() override;
		void VClearRenderTextures() override;

	private:
		ID3D11DepthStencilView*					mDepthStencilView;
		std::vector<ID3D11RenderTargetView*>	mRenderTargetViews;
		std::vector<ID3D11Texture2D*>			mRenderTextures;
	};
}