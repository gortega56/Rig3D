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
	class DXD11Renderer;

	class RIG3D DX11Mesh : public IMesh
	{
	public:
		DX11Mesh();
		~DX11Mesh();

	protected:
		ID3D11Buffer*	mVertexBuffer;
		ID3D11Buffer*	mIndexBuffer;
		
		friend class DX3D11Renderer;
	};
}