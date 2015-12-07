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
#include "Rig3D/Intersection.h"
#include "Rig3D/Visibility.h"
#include "Rig3D/Graphics/Camera.h"
#include "Rig3D/Geometry.h"
#include <d3d11.h>

#define INSTANCE_COUNT				1
#define SCENE_MEMORY				2048
#define PI							3.1415926535f
#define CAMERA_SPEED				0.1f
#define CAMERA_ROTATION_SPEED		0.1f
#define RADIAN						3.1415926535f / 180.0f

#define TERRAIN_PATCH_WIDTH_COUNT	5
#define TERRAIN_PATCH_DEPTH_COUNT	5
#define TERRAIN_WIDTH				10.0f
#define TERRAIN_DEPTH				10.0f
#define TERRAIN_VERTEX_DENSITY		5

using namespace Rig3D;

static uint32_t gSceneMemory[SCENE_MEMORY];

struct Vertex3
{
	vec3f Position;
	vec3f Normal;
	vec2f UV;
};

struct ViewProjection
{
	mat4f view;
	mat4f projection;
};

struct Terrain
{
	float width;
	float depth;
	float patchWidthCount;
	float patchDepthCount;
};

class SurfaceConstrainedMotionSample : public IScene, public IRendererDelegate
{
public:
	ViewProjection		mViewProjection;
	Terrain				mTerrain;
	Camera				mCamera;
	Plane<vec3f>		mPlanes[6];
	float				mMouseX;
	float				mMouseY;

	LinearAllocator		mLinearAllocator;

	mat4f*				mTerrainWorldMatrices;

	IRenderer*			mRenderer;
	IShader*			mTerrainVertexShader;
	IShader*			mTerrainPixelShader;
	IShader*			mCapsuleVertexShader;
	IShader*			mCapsulePixelShader;
	IShaderResource*	mShaderResouce;
	IMesh*				mTerrainMesh;
	IMesh*				mCapsuleMesh;

	SurfaceConstrainedMotionSample();
	~SurfaceConstrainedMotionSample();

	void VInitialize() override;
	void VUpdate(double milliseconds) override;
	void VRender() override;
	void VShutdown() override;

	void VOnResize() override;

	void InitializeGeometry();
	void InitializePhysics();
	void InitializeShaders();
	void InitializeShaderResources();

	void UpdateShaderResources();
	void UpdateInput(Input& input);
	void UpdateCamera();

	void Integrate(Transform* transforms, uint32_t count);
	void RK4(Transform* transforms, uint32_t count);
	void Euler(Transform* transforms, uint32_t count);

};

SurfaceConstrainedMotionSample::SurfaceConstrainedMotionSample() : 
	mMouseX(0.0f),
	mMouseY(0.0f),
	mLinearAllocator(gSceneMemory, gSceneMemory + SCENE_MEMORY),
	mTerrainWorldMatrices(nullptr),
	mRenderer(nullptr),
	mTerrainVertexShader(nullptr),
	mTerrainPixelShader(nullptr),
	mCapsuleVertexShader(nullptr),
	mCapsulePixelShader(nullptr),
	mShaderResouce(nullptr),
	mTerrainMesh(nullptr),
	mCapsuleMesh(nullptr)
{
	mOptions.mWindowCaption = "Surface Constrained Motion Sample";
	mOptions.mWindowWidth = 1200;
	mOptions.mWindowHeight = 900;
	mOptions.mGraphicsAPI = GRAPHICS_API_DIRECTX11;
	mOptions.mFullScreen = false;
}

SurfaceConstrainedMotionSample::~SurfaceConstrainedMotionSample()
{
}

void SurfaceConstrainedMotionSample::VInitialize()
{
	mRenderer = &DX3D11Renderer::SharedInstance();
	mRenderer->SetDelegate(this);

	mCamera.mTransform.SetPosition({ 0.0f, 10.0f, -10.0f });
	mCamera.mTransform.RotatePitch(30.0f * RADIAN);

	InitializeGeometry();
	InitializePhysics();
	InitializeShaders();
	InitializeShaderResources();

	VOnResize();
}

void SurfaceConstrainedMotionSample::InitializeGeometry()
{
	mTerrain.width = TERRAIN_WIDTH;
	mTerrain.depth = TERRAIN_DEPTH;
	mTerrain.patchWidthCount = static_cast<float>(TERRAIN_PATCH_WIDTH_COUNT);
	mTerrain.patchDepthCount = static_cast<float>(TERRAIN_PATCH_DEPTH_COUNT);

	float patchWidth = mTerrain.width / mTerrain.patchWidthCount;
	float patchDepth = mTerrain.depth / mTerrain.patchDepthCount;

	float halfWidth = (mTerrain.width - patchWidth) * 0.5f;
	float halfDepth = (mTerrain.depth - patchDepth) * 0.5f;

	mTerrainWorldMatrices = reinterpret_cast<mat4f*>(mLinearAllocator.Allocate(sizeof(mat4f) * TERRAIN_PATCH_WIDTH_COUNT * TERRAIN_PATCH_DEPTH_COUNT, alignof(mat4f), 0));

	for (uint32_t z = 0; z < TERRAIN_PATCH_DEPTH_COUNT; z++)
	{
		for (uint32_t x = 0; x < TERRAIN_PATCH_WIDTH_COUNT; x++)
		{
			mTerrainWorldMatrices[(z * TERRAIN_PATCH_WIDTH_COUNT) + x] = 
				mat4f::translate({ x * patchWidth - halfWidth, 0.0f, z * patchDepth - halfDepth }).transpose();
		}
	}

	std::vector<Vertex3> vertices;
	std::vector<uint16_t> indices;

	Geometry::Plane(vertices, indices, patchWidth, patchDepth, TERRAIN_VERTEX_DENSITY, TERRAIN_VERTEX_DENSITY);

	MeshLibrary<LinearAllocator> meshLibrary(&mLinearAllocator);
	meshLibrary.NewMesh(&mTerrainMesh, mRenderer);
	mRenderer->VSetMeshVertexBuffer(mTerrainMesh, &vertices[0], sizeof(Vertex3) * vertices.size(), sizeof(Vertex3));
	mRenderer->VSetMeshIndexBuffer(mTerrainMesh, &indices[0], indices.size());

	meshLibrary.NewMesh(&mCapsuleMesh, mRenderer);
}

void SurfaceConstrainedMotionSample::InitializePhysics()
{
}

void SurfaceConstrainedMotionSample::InitializeShaders()
{
	mRenderer->VCreateShader(&mTerrainVertexShader, &mLinearAllocator);
	mRenderer->VCreateShader(&mTerrainPixelShader, &mLinearAllocator);

	InputElement terrainInputElements[] =
	{
		{ "POSITION",	0, 0, 0, 0, RGB_FLOAT32, INPUT_CLASS_PER_VERTEX },
		{ "NORMAL",		0, 0, 12, 0, RGB_FLOAT32, INPUT_CLASS_PER_VERTEX },
		{ "TEXCOORD",	0, 0, 24, 0, RG_FLOAT32, INPUT_CLASS_PER_VERTEX },
		{ "WORLD",		0, 1, 0, 1, RGBA_FLOAT32, INPUT_CLASS_PER_INSTANCE },
		{ "WORLD",		1, 1, 16, 1, RGBA_FLOAT32, INPUT_CLASS_PER_INSTANCE },
		{ "WORLD",		2, 1, 32, 1, RGBA_FLOAT32, INPUT_CLASS_PER_INSTANCE },
		{ "WORLD",		3, 1, 48, 1, RGBA_FLOAT32, INPUT_CLASS_PER_INSTANCE }
	};

	mRenderer->VLoadVertexShader(mTerrainVertexShader, "TerrainVertexShader.cso", terrainInputElements, 7);
	mRenderer->VLoadPixelShader(mTerrainPixelShader, "TerrainPixelShader.cso");

	//mRenderer->VCreateShader(&mCapsuleVertexShader, &mLinearAllocator);
	//mRenderer->VCreateShader(&mCapsulePixelShader, &mLinearAllocator);

	//InputElement capsuleInputElements[] =
	//{
	//	{ "POSITION",	0, 0, 0, 0, RGB_FLOAT32, INPUT_CLASS_PER_VERTEX },
	//	{ "NORMAL",		0, 0, 12, 0, RGB_FLOAT32, INPUT_CLASS_PER_VERTEX },
	//	{ "TEXCOORD",	0, 0, 24, 0, RG_FLOAT32, INPUT_CLASS_PER_VERTEX }
	//};

	//mRenderer->VLoadVertexShader(mCapsuleVertexShader, "CapsuleVertexShader.cso", capsuleInputElements, 3);
	//mRenderer->VLoadPixelShader(mCapsulePixelShader, "CapsulePixelShader.cso");
}

void SurfaceConstrainedMotionSample::InitializeShaderResources()
{
	mRenderer->VCreateShaderResource(&mShaderResouce, &mLinearAllocator);

	// Instance buffer
	void*	instanceData[] = { &mTerrainWorldMatrices };
	size_t	instanceDataSizes[] = { sizeof(mat4f) * TERRAIN_PATCH_WIDTH_COUNT * TERRAIN_PATCH_DEPTH_COUNT };
	size_t	instanceDataStrides[] = { sizeof(mat4f) };
	size_t	instanceDataOffsets[] = { 0 };

	mRenderer->VCreateDynamicShaderInstanceBuffers(mShaderResouce, instanceData, instanceDataSizes, instanceDataStrides, instanceDataOffsets, 1);
	mRenderer->VUpdateShaderInstanceBuffer(mShaderResouce, mTerrainWorldMatrices, instanceDataSizes[0], 0);

	// Constant buffer
	void*	constantData[] = { &mViewProjection, &mTerrain };
	size_t	constantDataSize[] = { sizeof(ViewProjection), sizeof(Terrain)};

	mRenderer->VCreateShaderConstantBuffers(mShaderResouce, constantData, constantDataSize, 2);
	mRenderer->VUpdateShaderConstantBuffer(mShaderResouce, &mTerrain, 1);

	// Height map
	const char* filename = "Textures\\mt-tarawera-15m-dem.png";
	mRenderer->VCreateShaderTextures2D(mShaderResouce, &filename, 1);

	// Sampler
	mRenderer->VAddShaderLinearSamplerState(mShaderResouce, SAMPLER_STATE_ADDRESS_CLAMP);
}

void SurfaceConstrainedMotionSample::VUpdate(double milliseconds)
{
	UpdateInput(Input::SharedInstance());
	UpdateCamera();
	UpdateShaderResources();
}

void SurfaceConstrainedMotionSample::UpdateShaderResources()
{
	mViewProjection.projection = mCamera.GetProjectionMatrix().transpose();
	mViewProjection.view = mCamera.GetViewMatrix().transpose();
}

void SurfaceConstrainedMotionSample::UpdateInput(Input& input)
{
	ScreenPoint mousePosition = Input::SharedInstance().mousePosition;
	if (input.GetMouseButton(MOUSEBUTTON_LEFT))
	{
		mCamera.mTransform.RotatePitch(-(mousePosition.y - mMouseY) * RADIAN * CAMERA_ROTATION_SPEED);
		mCamera.mTransform.RotateYaw(-(mousePosition.x - mMouseX) * RADIAN * CAMERA_ROTATION_SPEED);
	}

	mMouseX = mousePosition.x;
	mMouseY = mousePosition.y;

	vec3f position = mCamera.mTransform.GetPosition();
	if (input.GetKey(KEYCODE_W))
	{
		position += mCamera.mTransform.GetForward() * CAMERA_SPEED;
		mCamera.mTransform.SetPosition(position);
	}

	if (input.GetKey(KEYCODE_A))
	{
		position += mCamera.mTransform.GetRight() * -CAMERA_SPEED;
		mCamera.mTransform.SetPosition(position);
	}

	if (input.GetKey(KEYCODE_D))
	{
		position += mCamera.mTransform.GetRight() * CAMERA_SPEED;
		mCamera.mTransform.SetPosition(position);
	}

	if (input.GetKey(KEYCODE_S))
	{
		position += mCamera.mTransform.GetForward() * -CAMERA_SPEED;
		mCamera.mTransform.SetPosition(position);
	}
}

void SurfaceConstrainedMotionSample::UpdateCamera()
{
	mCamera.SetViewMatrix(mat4f::lookAtLH(mCamera.mTransform.GetPosition() + mCamera.mTransform.GetForward(), mCamera.mTransform.GetPosition(), vec3f(0.0f, 1.0f, 0.0f)));
}

void SurfaceConstrainedMotionSample::Integrate(Transform* transforms, uint32_t count)
{
}

void SurfaceConstrainedMotionSample::RK4(Transform* transforms, uint32_t count)
{
}

void SurfaceConstrainedMotionSample::Euler(Transform* transforms, uint32_t count)
{
}

void SurfaceConstrainedMotionSample::VRender()
{
	float color[4] = { 0.0f, 0.7294117647f, 1.0f, 1.0f };

	mRenderer->VSetContextTargetWithDepth();
	mRenderer->VClearContext(color, 1.0f, 0);

	mRenderer->VSetPrimitiveType(GPU_PRIMITIVE_TYPE_TRIANGLE);
	mRenderer->VSetInputLayout(mTerrainVertexShader);
	mRenderer->VSetVertexShader(mTerrainVertexShader);
	mRenderer->VSetPixelShader(mTerrainPixelShader);

	mRenderer->VBindMesh(mTerrainMesh);

	mRenderer->VUpdateShaderConstantBuffer(mShaderResouce, &mViewProjection, 0);
	mRenderer->VSetVertexShaderInstanceBuffers(mShaderResouce);
	mRenderer->VSetVertexShaderConstantBuffers(mShaderResouce);
	mRenderer->VSetVertexShaderResourceView(mShaderResouce, 0, 0);
	mRenderer->VSetVertexShaderSamplerStates(mShaderResouce);

	ID3D11DeviceContext* deviceContext = static_cast<DX3D11Renderer*>(mRenderer)->GetDeviceContext();
	deviceContext->DrawIndexedInstanced(mTerrainMesh->GetIndexCount(), TERRAIN_PATCH_WIDTH_COUNT * TERRAIN_PATCH_DEPTH_COUNT, 0, 0, 0);

	mRenderer->VSwapBuffers();
}

void SurfaceConstrainedMotionSample::VShutdown()
{
	mTerrainMesh->~IMesh();
	mCapsuleMesh->~IMesh();
	mTerrainVertexShader->~IShader();
	mTerrainPixelShader->~IShader();
	//mCapsuleVertexShader->~IShader();
	//mCapsulePixelShader->~IShader();
	mShaderResouce->~IShaderResource();
	mLinearAllocator.Free();
}

void SurfaceConstrainedMotionSample::VOnResize()
{
	mCamera.SetProjectionMatrix(mat4f::normalizedPerspectiveLH(PI * 0.25f, mRenderer->GetAspectRatio(), 0.1f, 100.f));
}

DECLARE_MAIN(SurfaceConstrainedMotionSample);