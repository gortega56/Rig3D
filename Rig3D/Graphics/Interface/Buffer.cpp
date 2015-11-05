#include "Buffer.h"
#include "Graphics\DirectX11\DX3D11Renderer.h"

using namespace Rig3D;

GPUBuffer::GPUBuffer() : mDX11Buffer(nullptr)
{

}

GPUBuffer::~GPUBuffer()
{
	ReleaseMacro(mDX11Buffer);
}