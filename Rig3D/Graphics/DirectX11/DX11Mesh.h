#pragma once
#include "Rig3D\Graphics\Interface\IMesh.h"
#include <d3d11.h>

#ifdef _WINDLL
#define RIG3D __declspec(dllexport)
#else
#define RIG3D __declspec(dllimport)
#endif

namespace Rig3D
{
	class RIG3D DX11Mesh : public IMesh
	{
	public:
		DX11Mesh();
		~DX11Mesh();

		void VSetVertexBuffer(void* vertices, const size_t& size, const size_t& stride, const GPU_MEMORY_USAGE& usage);
		void VSetIndexBuffer(uint16_t* indices, const uint32_t& count, const GPU_MEMORY_USAGE& usage);
		void VBindVertexBuffer();
		void VBindIndexBuffer();

	private:
		ID3D11Buffer*	mVertexBuffer;
		ID3D11Buffer*	mIndexBuffer;
	};
}