#pragma once
#include "Rig3D/Graphics/Interface/IShader.h"
#include <map>
#include <d3d11.h>

namespace Rig3D
{
	class DXD11Renderer;

	struct DX11ShaderVariable
	{
		uint32_t ByteOffset;
		uint32_t Size;
		uint32_t BufferIndex;
	};

	struct DX11ShaderBuffer
	{
		ID3D11Buffer*	ConstantBuffer;
		void*			Data;
		uint8_t			BindIndex;
	};

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

		inline void SetVertexShader(ID3D11Device* device, ID3DBlob* vsBlob)
		{
			device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &mVertexShader);
		}

		inline void SetInputLayout(ID3D11Device* device, ID3DBlob* vsBlob, D3D11_INPUT_ELEMENT_DESC* inputLayoutDesc, UINT inputElementCount)
		{
			device->CreateInputLayout(inputLayoutDesc, inputElementCount, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &mInputLayout);
		}

		inline void SetBuffers(DX11ShaderBuffer* buffers, uint32_t bufferCount)
		{
			mBuffers = buffers;
			mBufferCount = mBufferCount;
		}

		inline DX11ShaderBuffer* GetShaderBufferAtIndex(uint32_t index)
		{
			return &mBuffers[index];
		}

		//inline ID3D11Buffer** GetBufferAtIndex(uint32_t index)
		//{
		//	return &mBuffers[index].ConstantBuffer;
		//}

		inline void SetPixelShader(ID3D11Device* device, ID3DBlob* psBlob)
		{
			device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &mPixelShader);
		}

		inline void UpdateConstantBufferForKey(ID3D11DeviceContext* deviceContext, const char* key, void* data)
		{
			deviceContext->UpdateSubresource(mConstantBufferMap.at(key), 0, nullptr, data, 0, 0);
		}

		inline void SetVSResources(ID3D11DeviceContext* deviceContext) const
		{
			
		}

		inline void SetPSResources(ID3D11DeviceContext* deviceContext) const
		{
			
		}

	private:
		uint32_t								mBufferCount;

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

		DX11ShaderBuffer*						mBuffers;

		std::map<const char*, ID3D11Buffer*>	mConstantBufferMap;
		std::map<const char*, UINT>				mShaderResourceViewMap;
		std::map<const char*, UINT>				mSamplerStateMap;
	};
}