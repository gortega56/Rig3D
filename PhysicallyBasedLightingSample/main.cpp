#include <Windows.h>
#include "Rig3D\Engine.h"
#include "Rig3D\Graphics\Interface\IScene.h"
#include "Rig3D\Graphics\DirectX11\DX3D11Renderer.h"
#include "Rig3D\Graphics\Interface\IMesh.h"
#include "Rig3D\Common\Transform.h"
#include "Memory\Memory\Memory.h"
#include "Rig3D\Graphics\MeshLibrary.h"
#include "Rig3D\Graphics\Interface\IShader.h"
#include "Rig3D\Graphics\Interface\IShaderResource.h"
#include "Rig3D/Graphics/Camera.h"
#include <d3d11.h>
#include <Rig3D\Graphics\DirectX11\DirectXTK\Inc\DDSTextureLoader.h>

#define PI						3.1415926535f
#define CAMERA_SPEED			0.1f
#define CAMERA_ROTATION_SPEED	0.1f
#define RADIAN					3.1415926535f / 180.0f
#define INSTANCE_COUNT			1

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
	vec4f	CameraPosition;
};

struct ModelData
{
	mat4f View;
	mat4f Projection;
};

class PhysicallyBasedLightingSample : public IScene, public virtual IRendererDelegate
{
public:
	mat4f						mSphereWorldMatrices[INSTANCE_COUNT];
	SkyboxData					mSkyboxData;
	ModelData					mModelData;

	Camera						mCamera;
	LinearAllocator				mAllocator;

	Transform*					mTransforms;
	IMesh*						mSkyboxMesh;
	IMesh*						mIcosphereMesh;

	IRenderer*					mRenderer;
	IShader*					mPBLSkyboxVertexShader;
	IShader*					mPBLSkyboxPixelShader;
	IShader*					mPBLModelVertexShader;
	IShader*					mPBLModelPixelShader;
	IShaderResource*			mPBLSkyboxShaderResource;
	IShaderResource*			mPBLModelShaderResource;

	float				mMouseX;
	float				mMouseY;

	ID3D11DepthStencilState*	mDepthStencilState;

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
	mPBLSkyboxShaderResource(nullptr),
	mPBLModelShaderResource(nullptr),
	mMouseX(0.0f),
	mMouseY(0.0f),
	mDepthStencilState(nullptr)
{
	mOptions.mWindowCaption = "Physically Based Lighting Sample";
	mOptions.mWindowWidth = 1200;
	mOptions.mWindowHeight = 900;
	mOptions.mGraphicsAPI = GRAPHICS_API_DIRECTX11;
	mOptions.mFullScreen = false;
}

PhysicallyBasedLightingSample::~PhysicallyBasedLightingSample()
{
	ReleaseMacro(mDepthStencilState);
}

void PhysicallyBasedLightingSample::VInitialize()
{
	mRenderer = &DX3D11Renderer::SharedInstance();
	mRenderer->SetDelegate(this);

	InitializeGeometry();
	InitializeShaders();
	InitializeShaderResources();

	VOnResize();
}

void PhysicallyBasedLightingSample::VUpdate(double milliseconds)
{
	HandleInput(Input::SharedInstance());

	mSkyboxData.Model = mat4f::translate(mCamera.mTransform.GetPosition()).transpose();
	mSkyboxData.Projection = mModelData.Projection = mCamera.GetProjectionMatrix().transpose();
	mSkyboxData.View = mModelData.View = mCamera.GetViewMatrix().transpose();
	mSkyboxData.CameraPosition = mCamera.mTransform.GetPosition();

	//mRenderer->VUpdateShaderInstanceBuffer(mPBLModelVertexShader, &mSphereWorldMatrices, sizeof(mat4f) * INSTANCE_COUNT, 0);
}

void PhysicallyBasedLightingSample::VRender()
{
	ID3D11DeviceContext* deviceContext	= static_cast<DX3D11Renderer*>(mRenderer)->GetDeviceContext();
	DX3D11Renderer*	renderer			= reinterpret_cast<DX3D11Renderer*>(mRenderer);

	float color[4] = { 1.0f, 1.0, 1.0f, 1.0f };
	mRenderer->VSetContextTargetWithDepth();
	mRenderer->VClearContext(color, 1.0f, 0);
	deviceContext->RSSetViewports(1, &renderer->GetViewport());

	deviceContext->OMSetDepthStencilState(nullptr, 1);

	mRenderer->VSetPrimitiveType(GPU_PRIMITIVE_TYPE_TRIANGLE);
	
	mRenderer->VSetInputLayout(mPBLModelVertexShader);
	mRenderer->VSetVertexShader(mPBLModelVertexShader);
	mRenderer->VSetPixelShader(mPBLModelPixelShader);

	mRenderer->VUpdateShaderConstantBuffer(mPBLModelShaderResource, &mModelData, 0);
	mRenderer->VSetVertexShaderConstantBuffer(mPBLModelShaderResource, 0, 0);

	mRenderer->VBindMesh(mIcosphereMesh);
	mRenderer->VSetVertexShaderInstanceBuffers(mPBLModelShaderResource);

	deviceContext->DrawIndexedInstanced(mIcosphereMesh->GetIndexCount(), INSTANCE_COUNT, 0, 0, 0);

	deviceContext->OMSetDepthStencilState(mDepthStencilState, 1);

	mRenderer->VSetInputLayout(mPBLSkyboxVertexShader);

	mRenderer->VSetVertexShader(mPBLSkyboxVertexShader);
	mRenderer->VSetPixelShader(mPBLSkyboxPixelShader);

	mRenderer->VUpdateShaderConstantBuffer(mPBLSkyboxShaderResource, &mSkyboxData, 0);
	mRenderer->VSetVertexShaderConstantBuffer(mPBLSkyboxShaderResource, 0, 0);

	mRenderer->VSetPixelShaderResourceViews(mPBLSkyboxShaderResource);
	mRenderer->VSetPixelShaderSamplerStates(mPBLSkyboxShaderResource);

	mRenderer->VBindMesh(mSkyboxMesh);
	mRenderer->VDrawIndexed(0, mSkyboxMesh->GetIndexCount());

	mRenderer->VSwapBuffers();
}

void PhysicallyBasedLightingSample::VShutdown()
{
	mSkyboxMesh->~IMesh();
	mIcosphereMesh->~IMesh();
	mPBLModelVertexShader->~IShader();
	mPBLModelPixelShader->~IShader();
	mPBLSkyboxVertexShader->~IShader();
	mPBLSkyboxPixelShader->~IShader();
	mPBLSkyboxShaderResource->~IShaderResource();
	mPBLModelShaderResource->~IShaderResource();
	mAllocator.Free();
}

void PhysicallyBasedLightingSample::VOnResize()
{
	mCamera.SetProjectionMatrix(mat4f::normalizedPerspectiveLH(0.25f * PI, mRenderer->GetAspectRatio(), 0.1f, 100.0f));
}

void PhysicallyBasedLightingSample::InitializeGeometry()
{
	OBJBasicResource<Vertex3> skyboxResource("Models\\skybox.obj");
	OBJBasicResource<Vertex3> icosahedronResource("Models\\icosahedron.obj");

	MeshLibrary<LinearAllocator> meshLibrary(&mAllocator);
	meshLibrary.LoadMesh(&mSkyboxMesh, mRenderer, skyboxResource);
	meshLibrary.LoadMesh(&mIcosphereMesh, mRenderer, icosahedronResource);

	for (uint32_t i = 0; i < INSTANCE_COUNT; i++)
	{
		mSphereWorldMatrices[i] = (mat4f::scale(0.2f) * mat4f::translate(0.2f)).transpose();
	}
}

void PhysicallyBasedLightingSample::InitializeShaders()
{
	InputElement skyboxInputElements[] =
	{
		{ "POSITION",	0, 0, 0, 0, RGB_FLOAT32, INPUT_CLASS_PER_VERTEX },
		{ "NORMAL",		0, 0, 12, 0, RGB_FLOAT32, INPUT_CLASS_PER_VERTEX },
		{ "TEXCOORD",	0, 0, 24, 0, RG_FLOAT32, INPUT_CLASS_PER_VERTEX }
	};

	mRenderer->VCreateShader(&mPBLSkyboxVertexShader, &mAllocator);
	mRenderer->VLoadVertexShader(mPBLSkyboxVertexShader, "PBLSkyboxVertexShader.cso", skyboxInputElements, 3);

	mRenderer->VCreateShader(&mPBLSkyboxPixelShader, &mAllocator);
	mRenderer->VLoadPixelShader(mPBLSkyboxPixelShader, "PBLSkyboxPixelShader.cso");

	InputElement sphereInputElements[] =
	{
		{ "POSITION",	0, 0, 0, 0, RGB_FLOAT32, INPUT_CLASS_PER_VERTEX },
		{ "NORMAL",		0, 0, 12, 0, RGB_FLOAT32, INPUT_CLASS_PER_VERTEX },
		{ "TEXCOORD",	0, 0, 24, 0, RG_FLOAT32, INPUT_CLASS_PER_VERTEX },
		{ "WORLD",		0, 1, 0, 1, RGBA_FLOAT32, INPUT_CLASS_PER_INSTANCE },
		{ "WORLD",		1, 1, 16, 1, RGBA_FLOAT32, INPUT_CLASS_PER_INSTANCE },
		{ "WORLD",		2, 1, 32, 1, RGBA_FLOAT32, INPUT_CLASS_PER_INSTANCE },
		{ "WORLD",		3, 1, 48, 1, RGBA_FLOAT32, INPUT_CLASS_PER_INSTANCE }
	};

	mRenderer->VCreateShader(&mPBLModelVertexShader, &mAllocator);
	mRenderer->VLoadVertexShader(mPBLModelVertexShader, "PBLInstanceVertexShader.cso", sphereInputElements, 7);

	mRenderer->VCreateShader(&mPBLModelPixelShader, &mAllocator);
	mRenderer->VLoadPixelShader(mPBLModelPixelShader, "PBLInstancePixelShader.cso");
}

void PhysicallyBasedLightingSample::InitializeShaderResources()
{
	mRenderer->VCreateShaderResource(&mPBLSkyboxShaderResource, &mAllocator);

	size_t	skyboxConstantBufferSizes[] = { sizeof(SkyboxData) };
	void*	skyboxData[]				= { &mSkyboxData };
	mRenderer->VCreateShaderConstantBuffers(mPBLSkyboxShaderResource, skyboxData, skyboxConstantBufferSizes, 1);

	const char* skyboxes[] =
	{
		"Textures\\Skybox.dds"
	};

	mRenderer->VCreateShaderTextureCubes(mPBLSkyboxShaderResource, skyboxes, 1);
	mRenderer->VAddShaderLinearSamplerState(mPBLSkyboxShaderResource, SAMPLER_STATE_ADDRESS_WRAP);

	mRenderer->VCreateShaderResource(&mPBLModelShaderResource, &mAllocator);

	size_t	modelConstantBufferSizes[] = { sizeof(ModelData) };
	void*	modelData[] = { &mModelData };
	mRenderer->VCreateShaderConstantBuffers(mPBLModelShaderResource, modelData, modelConstantBufferSizes, 1);

	size_t instanceBufferSizes[] = { sizeof(mat4f) * INSTANCE_COUNT };
	UINT strides[] = { sizeof(mat4f) };
	UINT offsets[] = { 0 };
	void* data[] = { &mSphereWorldMatrices };
	mRenderer->VCreateShaderInstanceBuffers(mPBLModelShaderResource, data, instanceBufferSizes, strides, offsets, 1);

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	ID3D11Device* device = reinterpret_cast<DX3D11Renderer *>(mRenderer)->GetDevice();
	device->CreateDepthStencilState(&depthStencilDesc, &mDepthStencilState);
}

void PhysicallyBasedLightingSample::HandleInput(Input& input)
{
	ScreenPoint mousePosition = Input::SharedInstance().mousePosition;
	if (input.GetMouseButton(MOUSEBUTTON_LEFT)) {
		mCamera.mTransform.RotatePitch(-(mousePosition.y - mMouseY) * RADIAN * CAMERA_ROTATION_SPEED);
		mCamera.mTransform.RotateYaw(-(mousePosition.x - mMouseX) * RADIAN * CAMERA_ROTATION_SPEED);
	}

	mMouseX = mousePosition.x;
	mMouseY = mousePosition.y;

	vec3f position = mCamera.mTransform.GetPosition();
	if (input.GetKey(KEYCODE_W)) {
		position += mCamera.mTransform.GetForward() * CAMERA_SPEED;
		mCamera.mTransform.SetPosition(position);
	}

	if (input.GetKey(KEYCODE_A)) {
		position += mCamera.mTransform.GetRight() * -CAMERA_SPEED;
		mCamera.mTransform.SetPosition(position);
	}

	if (input.GetKey(KEYCODE_D)) {
		position += mCamera.mTransform.GetRight() * CAMERA_SPEED;
		mCamera.mTransform.SetPosition(position);
	}

	if (input.GetKey(KEYCODE_S)) {
		position += mCamera.mTransform.GetForward() * -CAMERA_SPEED;
		mCamera.mTransform.SetPosition(position);
	}
}

DECLARE_MAIN(PhysicallyBasedLightingSample);