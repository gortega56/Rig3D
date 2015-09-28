#include "DX11Mesh.h"
#include "DX3D11Renderer.h"

using namespace Rig3D;

DX11Mesh::DX11Mesh() : IMesh()
{

}

DX11Mesh::~DX11Mesh()
{
	ReleaseMacro(mVertexBuffer);
	ReleaseMacro(mIndexBuffer);
}
