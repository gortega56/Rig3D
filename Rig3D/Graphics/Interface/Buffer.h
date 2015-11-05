#pragma once
#include <d3d11.h>

#ifdef _WINDLL
#define RIG3D __declspec(dllexport)
#else
#define RIG3D __declspec(dllimport)
#endif

namespace Rig3D
{
	class RIG3D GPUBuffer
	{
	public:
		GPUBuffer();
		~GPUBuffer();

		inline ID3D11Buffer** GetDX11()
		{
			return &mDX11Buffer;
		}

		inline void GetGL(void* pointer)
		{

		}

		inline void SetDX11(void* pointer)
		{
			mDX11Buffer = reinterpret_cast<ID3D11Buffer*>(pointer);
		}

		inline void SetGL(void* pointer)
		{

		}

	private:
		union
		{
			ID3D11Buffer* mDX11Buffer;
		};
	};
}