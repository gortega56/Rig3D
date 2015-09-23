#include "IRenderer.h"
#include "Graphics\DirectX11\DX3D11Renderer.h"

using namespace Rig3D;

IRenderer::IRenderer()
{
	mDelegate = nullptr;
}

IRenderer::~IRenderer()
{

}
