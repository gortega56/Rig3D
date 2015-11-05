#pragma once
#include "Rig3D/Graphics/Interface/IShader.h"
#include <map>
#include <d3d11.h>

namespace Rig3D
{
	class DXD11Renderer;

	class DX11Shader : public IShader
	{
	public:
		DX11Shader();
		~DX11Shader();

		inline ID3D11VertexShader* GetVertexShader() const
		{
			return mVertexShader;
		}

		inline ID3D11InputLayout* GetInputLayout()
		{
			return mInputLayout;
		}

		inline ID3D11PixelShader* GetPixelShader() const
		{
			return mPixelShader;
		}

		inline void CreateVertexShader(ID3D11Device* device, ID3DBlob* vsBlob)
		{
			device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &mVertexShader);
		}

		inline void CreateInputLayout(ID3D11Device* device, ID3DBlob* vsBlob, D3D11_INPUT_ELEMENT_DESC* inputLayoutDesc, UINT inputElementCount)
		{
			device->CreateInputLayout(inputLayoutDesc, inputElementCount, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &mInputLayout);
		}

		inline void SetConstantBuffers(ID3D11Buffer** buffers, uint8_t count)
		{
			mBuffers = buffers;
			mBufferCount = count;
		}

		inline void SetShaderResourceViews(ID3D11ShaderResourceView** srvs, uint8_t count)
		{
			mShaderResourceViews = srvs;
			mShaderResourceViewCount = count;
		}

		inline void SetSamperStates(ID3D11SamplerState** samplerStates, uint8_t count)
		{
			mSamplerStates = samplerStates;
			mSamplerStateCount = count;
		}

		inline void CreatePixelShader(ID3D11Device* device, ID3DBlob* psBlob)
		{
			device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &mPixelShader);
		}

		inline void UpdateConstantBuffer(ID3D11DeviceContext* deviceContext, void* data, uint8_t index)
		{
			deviceContext->UpdateSubresource(mBuffers[index], 0, nullptr, data, 0, 0);
		}

		inline void SetVSResources(ID3D11DeviceContext* deviceContext) const
		{
			if (mBuffers)
			{
				deviceContext->VSSetConstantBuffers(0, mBufferCount, mBuffers);
			}

			if (mShaderResourceViews)
			{
				deviceContext->VSSetShaderResources(0, mShaderResourceViewCount, mShaderResourceViews);
			}

			if (mSamplerStates)
			{
				deviceContext->VSSetSamplers(0, mSamplerStateCount, mSamplerStates);
			}
		}

		inline void SetPSResources(ID3D11DeviceContext* deviceContext) const
		{
			
		}

	private:
		uint8_t					mBufferCount;
		uint8_t					mShaderResourceViewCount;
		uint8_t					mSamplerStateCount;

		union
		{
			struct
			{
				ID3D11VertexShader* mVertexShader;
				ID3D11InputLayout*	mInputLayout;
			};

			struct
			{
				ID3D11PixelShader*	mPixelShader;
				char padding[sizeof(void*)];
			};
		};

		ID3D11Buffer**				mBuffers;
		ID3D11ShaderResourceView**	mShaderResourceViews;
		ID3D11SamplerState**		mSamplerStates;
	};
}