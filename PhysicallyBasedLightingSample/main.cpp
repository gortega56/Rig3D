#include <Windows.h>
#include "Rig3D\Engine.h"
#include "Rig3D\Graphics\Interface\IScene.h"
#include "Rig3D\Graphics\DirectX11\DX3D11Renderer.h"
#include "Rig3D\Graphics\Interface\IMesh.h"
#include "Rig3D\Common\Transform.h"
#include "Memory\Memory\Memory.h"
#include "Rig3D\Graphics\MeshLibrary.h"
#include "Rig3D\Graphics\Interface\IShader.h"
#include <d3d11.h>
#include <Rig3D\Graphics\DirectX11\DirectXTK\Inc\DDSTextureLoader.h>

#define INSTANCE_COUNT	1

using namespace Rig3D;

uint8_t				gMemory[1024];

struct Vertex3
{
	vec3f Position;
	vec3f Normal;
	vec2f UV;
};

struct SkyboxData
{
	mat4f	Model;
	mat4f	View;
	mat4f	Projection;
};

class PhysicallyBasedLightingSample : public IScene, public virtual IRendererDelegate
{
public:
	mat4f						mSphereWorldMatrices[INSTANCE_COUNT];
	SkyboxData					mSkyboxData;

	Transform					mCamera;
	LinearAllocator				mAllocator;

	Transform*					mTransforms;
	IMesh*						mSkyboxMesh;
	IMesh*						mIcosphereMesh;

	IRenderer*					mRenderer;
	IShader*					mPBLSkyboxVertexShader;
	IShader*					mPBLSkyboxPixelShader;
	IShader*					mPBLModelVertexShader;
	IShader*					mPBLModelPixelShader;

	ID3D11ShaderResourceView*	mSkyboxSRV;

	float				mMouseX;
	float				mMouseY;

	PhysicallyBasedLightingSample();
	~PhysicallyBasedLightingSample();

	void VInitialize() override;
	void VUpdate(double milliseconds) override;
	void VRender() override;
	void VShutdown() override;
	void VOnResize() override;

	void InitializeGeometry();
	void InitializeShaders();
	void InitializeShaderResources();

	void UpdateCamera();
	void HandleInput(Input& input);
};

PhysicallyBasedLightingSample::PhysicallyBasedLightingSample() :
	mAllocator(gMemory, gMemory + 1024),
	mTransforms(nullptr),
	mSkyboxMesh(nullptr),
	mIcosphereMesh(nullptr),
	mRenderer(nullptr),
	mPBLSkyboxVertexShader(nullptr),
	mPBLSkyboxPixelShader(nullptr),
	mPBLModelVertexShader(nullptr),
	mPBLModelPixelShader(nullptr),
	mSkyboxSRV(nullptr),
	mMouseX(0.0f),
	mMouseY(0.0f)
{
	mOptions.mWindowCaption = "Physically Based Lighting Sample";
	mOptions.mWindowWidth = 1200;
	mOptions.mWindowHeight = 900;
	mOptions.mGraphicsAPI = GRAPHICS_API_DIRECTX11;
	mOptions.mFullScreen = false;
}

PhysicallyBasedLightingSample::~PhysicallyBasedLightingSample()
{
	ReleaseMacro(mSkyboxSRV);
}

void PhysicallyBasedLightingSample::VInitialize()
{
	mRenderer = &DX3D11Renderer::SharedInstance();

	InitializeGeometry();
	InitializeShaders();
	InitializeShaderResources();
}

void PhysicallyBasedLightingSample::VUpdate(double milliseconds)
{
	
}

void PhysicallyBasedLightingSample::VRender()
{
	
}

void PhysicallyBasedLightingSample::VShutdown()
{
	mSkyboxMesh->~IMesh();
	mIcosphereMesh->~IMesh();
	mPBLModelVertexShader->~IShader();
	mPBLModelPixelShader->~IShader();
	mAllocator.Free();
}

void PhysicallyBasedLightingSample::VOnResize()
{
	
}

void PhysicallyBasedLightingSample::InitializeGeometry()
{
	OBJBasicResource<Vertex3> skyboxResource("Models\\skybox.obj");
	OBJBasicResource<Vertex3> icosahedronResource("Models\\icosahedron.obj");

	MeshLibrary<LinearAllocator> meshLibrary(&mAllocator);
	meshLibrary.LoadMesh(&mSkyboxMesh, mRenderer, skyboxResource);
	meshLibrary.LoadMesh(&mIcosphereMesh, mRenderer, icosahedronResource);
}

void PhysicallyBasedLightingSample::InitializeShaders()
{
	InputElement skyboxInputElements[] =
	{
		{ "POSITION",	0, 0, 0, 0, FLOAT3, INPUT_CLASS_PER_VERTEX },
		{ "NORMAL",		0, 0, 12, 0, FLOAT3, INPUT_CLASS_PER_VERTEX },
		{ "TEXCOORD",	0, 0, 24, 0, FLOAT2, INPUT_CLASS_PER_VERTEX }
	};

	mRenderer->VCreateShader(&mPBLSkyboxVertexShader, &mAllocator);
	mRenderer->VLoadVertexShader(mPBLSkyboxVertexShader, "PBLSkyboxVertexShader.cso", skyboxInputElements, 3);

	mRenderer->VCreateShader(&mPBLSkyboxPixelShader, &mAllocator);
	mRenderer->VLoadPixelShader(mPBLSkyboxPixelShader, "PBLSkyboxPixelShader.cso");

	InputElement sphereInputElements[] =
	{
		{ "POSITION",	0, 0, 0, 0, FLOAT3, INPUT_CLASS_PER_VERTEX },
		{ "NORMAL",		0, 0, 12, 0, FLOAT3, INPUT_CLASS_PER_VERTEX },
		{ "TEXCOORD",	0, 0, 24, 0, FLOAT2, INPUT_CLASS_PER_VERTEX },
		{ "WORLD",		0, 1, 0, 1, FLOAT4, INPUT_CLASS_PER_INSTANCE },
		{ "WORLD",		1, 1, 16, 1, FLOAT4, INPUT_CLASS_PER_INSTANCE },
		{ "WORLD",		2, 1, 32, 1, FLOAT4, INPUT_CLASS_PER_INSTANCE },
		{ "WORLD",		3, 1, 48, 1, FLOAT4, INPUT_CLASS_PER_INSTANCE }
	};

	mRenderer->VCreateShader(&mPBLModelVertexShader, &mAllocator);
	mRenderer->VLoadVertexShader(mPBLModelVertexShader, "PBLInstanceVertexShader.cso", skyboxInputElements, 7);

	mRenderer->VCreateShader(&mPBLModelPixelShader, &mAllocator);
	mRenderer->VLoadPixelShader(mPBLModelPixelShader, "PBLInstancePixelShader.cso");
}

void PhysicallyBasedLightingSample::InitializeShaderResources()
{
	size_t skyboxConstantBufferSizes[] = { sizeof(SkyboxData) };
	mRenderer->VCreateShaderConstantBuffers(mPBLSkyboxVertexShader, nullptr, skyboxConstantBufferSizes, 1);

	ID3D11Device* device = static_cast<DX3D11Renderer*>(mRenderer)->GetDevice();
	DirectX::CreateDDSTextureFromFile(device, L"Textures\\Skybox.dds", nullptr, &mSkyboxSRV);

	size_t instanceBufferSizes[] = { sizeof(mat4f) * INSTANCE_COUNT };
	UINT strides[]	= { sizeof(mat4f) };
	UINT offsets[]	= { 0 };
	void* data[]	= { mSphereWorldMatrices };
	mRenderer->VCreateShaderInstanceBuffers(mPBLModelVertexShader, data, instanceBufferSizes, strides, offsets, 1);
}

void PhysicallyBasedLightingSample::UpdateCamera()
{
	
}

void PhysicallyBasedLightingSample::HandleInput(Input& input)
{
	
}