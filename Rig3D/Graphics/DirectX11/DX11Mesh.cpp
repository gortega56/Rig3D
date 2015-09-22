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

void DX11Mesh::VSetVertexBuffer(void* vertices, const size_t& size, const size_t& stride, const GPU_MEMORY_USAGE& usage)
{
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = GD3D11_Usage_Map(usage);
	vbd.ByteWidth = size;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;	// Not used for Vertex Input

	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;
	DX3D11Renderer::SharedInstance().GetDevice()->CreateBuffer(&vbd, &vertexData, &mVertexBuffer);

	mVertexStride = stride;
}

void DX11Mesh::VSetIndexBuffer(uint16_t* indices, const uint32_t& count, const GPU_MEMORY_USAGE& usage)
{
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = GD3D11_Usage_Map(usage);
	ibd.ByteWidth = sizeof(uint16_t) * count;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	DX3D11Renderer::SharedInstance().GetDevice()->CreateBuffer(&ibd, &indexData, &mIndexBuffer);

	mIndexCount = count;
}

void DX11Mesh::VBindVertexBuffer()
{
	uint32_t offset = 0;
	DX3D11Renderer::SharedInstance().GetDeviceContext()->IASetVertexBuffers(0, 1, &mVertexBuffer, &mVertexStride, &offset);
}

void DX11Mesh::VBindIndexBuffer()
{
	DX3D11Renderer::SharedInstance().GetDeviceContext()->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
}
