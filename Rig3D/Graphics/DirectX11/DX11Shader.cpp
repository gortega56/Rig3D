#include "DX11Shader.h"
#include "DX3D11Renderer.h"

using namespace Rig3D;

DX11Shader::DX11Shader() : 
	mBufferCount(0), 
	mShaderResourceViewCount(0), 
	mSamplerStateCount(0), 
	mVertexShader(nullptr),
	mInputLayout(nullptr),
	mBuffers(nullptr), 
	mShaderResourceViews(nullptr), 
	mSamplerStates(nullptr)
{
}


DX11Shader::~DX11Shader()
{
	ReleaseMacro(mVertexShader);
	ReleaseMacro(mInputLayout);
	ReleaseMacro(mPixelShader);

	//for (std::map<const char*, ID3D11Buffer*>::iterator iterator = mConstantBufferMap.begin(); iterator != mConstantBufferMap.end(); ++iterator)
	//{
	//	ReleaseMacro(iterator->second);
	//}
}