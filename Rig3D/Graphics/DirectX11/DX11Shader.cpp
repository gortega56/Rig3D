#include "DX11Shader.h"
#include "DX3D11Renderer.h"

using namespace Rig3D;

DX11Shader::DX11Shader() : mVertexShader(nullptr)
{
}


DX11Shader::~DX11Shader()
{
	ReleaseMacro(mVertexShader);
	ReleaseMacro(mPixelShader);

	for (std::map<const char*, ID3D11Buffer*>::iterator iterator = mConstantBufferMap.begin(); iterator != mConstantBufferMap.end(); ++iterator)
	{
		ReleaseMacro(iterator->second);
	}
}