#include <Windows.h>
#include "Rig3D\Engine.h"
#include "Rig3D\Graphics\Interface\IScene.h"
#include "Rig3D\Graphics\DirectX11\DX3D11Renderer.h"
#include "Rig3D\Graphics\Interface\IMesh.h"
#include "Rig3D\Common\Transform.h"
#include "Memory\Memory\LinearAllocator.h"
#include "Rig3D\Graphics\DirectX11\DirectXTK\Inc\WICTextureLoader.h"
#include "Rig3D\MeshLibrary.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <Rig3D/Graphics/Camera.h>

#define BALL_COUNT 16

using namespace Rig3D;

static const int gMeshMemorySize = 2048;

char gMeshMemory[gMeshMemorySize];

class BilliardsSample : public IScene, public virtual IRendererDelegate
{
public:
	typedef cliqCity::memory::LinearAllocator LinearAllocator;

	struct Vertex3
	{
		vec3f position;
		vec3f normal;
		vec2f uv;
	};

	struct PoolTable
	{
		IMesh* legs;
		IMesh* feet;
		IMesh* sides;
		IMesh* guards;
		IMesh* surface;
		IMesh* bottom;
		IMesh* holes;
	};

	struct MatrixBuffer
	{
		mat4f view;
		mat4f projection;
	};

	MatrixBuffer	mMatrixBuffer;
	mat4f			mBallTransforms[16];
	Camera			mCamera;

	LinearAllocator					mAllocator;
	MeshLibrary<LinearAllocator>	mMeshLibrary;

	DXD11Renderer*					mRenderer;
	ID3D11DeviceContext*			mDeviceContext;
	ID3D11Device*					mDevice;

	PoolTable	mPoolTable;
	IMesh*		mBallMesh;

	ID3D11InputLayout*	mBallInputLayout;
	ID3D11VertexShader* mBallVertexShader;
	ID3D11PixelShader*	mPixelShader;

	ID3D11ShaderResourceView* mBallSRVs[16];	


	BilliardsSample() : 
		mAllocator(gMeshMemory, gMeshMemory + gMeshMemorySize),
		mRenderer(nullptr),
		mDeviceContext(nullptr),
		mDevice(nullptr),
		mBallMesh(nullptr), 
		mBallInputLayout(nullptr),
		mBallVertexShader(nullptr),
		mPixelShader(nullptr)
	{
		mOptions.mWindowCaption = "Deferred Lighting Sample";
		mOptions.mWindowWidth = 800;
		mOptions.mWindowHeight = 600;
		mOptions.mGraphicsAPI = GRAPHICS_API_DIRECTX11;
		mOptions.mFullScreen = false;
		mMeshLibrary.SetAllocator(&mAllocator);
	}

	~BilliardsSample()
	{
		
	}

	void VInitialize()	override
	{
		mRenderer = &DX3D11Renderer::SharedInstance();
		mRenderer->SetDelegate(this);

		mDevice = mRenderer->GetDevice();
		mDeviceContext = mRenderer->GetDeviceContext();

		InitializeGeometry();
		InitializeShaders();
		InitializeShaderResources();
		InitializeCamera();
	}

	void VUpdate(double milliseconds) override
	{

	}
	
	void VRender() override
	{

	}

	void VShutdown() override
	{

	}

	void VOnResize() override
	{

	}

	void InitializeGeometry()
	{
		OBJResource<Vertex3> poolTableLegs("Models\\Legs.obj");
		OBJResource<Vertex3> poolTableFeet("Models\\LegsMetalPart.obj");
		OBJResource<Vertex3> poolTableSides("Models\\Side.obj");
		OBJResource<Vertex3> poolTableGuards("Models\\SideGuard.obj");
		OBJResource<Vertex3> poolTableSurface("Models\\TopSurface.obj");
		OBJResource<Vertex3> poolTableBottom("Models\\Bottom.obj");
		OBJResource<Vertex3> poolTableHoles("Models\\Holes.obj");
		OBJResource<Vertex3> ball("Models\\Ball.obj");

		mMeshLibrary.LoadMesh(&mPoolTable.legs, mRenderer, poolTableLegs);
		mMeshLibrary.LoadMesh(&mPoolTable.feet, mRenderer, poolTableLegs);
		mMeshLibrary.LoadMesh(&mPoolTable.sides, mRenderer, poolTableLegs);
		mMeshLibrary.LoadMesh(&mPoolTable.guards, mRenderer, poolTableLegs);
		mMeshLibrary.LoadMesh(&mPoolTable.surface, mRenderer, poolTableLegs);
		mMeshLibrary.LoadMesh(&mPoolTable.bottom, mRenderer, poolTableLegs);
		mMeshLibrary.LoadMesh(&mPoolTable.holes, mRenderer, poolTableLegs);
		mMeshLibrary.LoadMesh(&mBallMesh, mRenderer, ball);
	}

	void InitializeShaders()
	{
		D3D11_INPUT_ELEMENT_DESC inputDescription[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,	0, 24,D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
		};

		ID3DBlob* vsBlob;
		D3DReadFileToBlob(L"BallVertexShader.cso", &vsBlob);

		// Create the shader on the device
		mDevice->CreateVertexShader(
			vsBlob->GetBufferPointer(),
			vsBlob->GetBufferSize(),
			nullptr,
			&mBallVertexShader);

		// Before cleaning up the data, create the input layout
		if (inputDescription) {
			mDevice->CreateInputLayout(
				inputDescription,					// Reference to Description
				5,									// Number of elments inside of Description
				vsBlob->GetBufferPointer(),
				vsBlob->GetBufferSize(),
				&mBallInputLayout);
		}

		// Clean up
		vsBlob->Release();
	}

	void InitializeShaderResources()
	{
		const char* ballTextures[BALL_COUNT] = {
			"Textures\\pallina0.jpg",
			"Textures\\pallina1.jpg",
			"Textures\\pallina2.jpg",
			"Textures\\pallina3.jpg",
			"Textures\\pallina4.jpg",
			"Textures\\pallina5.jpg",
			"Textures\\pallina6.jpg",
			"Textures\\pallina7.jpg",
			"Textures\\pallina8.jpg",
			"Textures\\pallina9.jpg",
			"Textures\\pallina10.jpg",
			"Textures\\pallina11.jpg",
			"Textures\\pallina12.jpg",
			"Textures\\pallina13.jpg",
			"Textures\\pallina14.jpg",
			"Textures\\pallina15.jpg"
		};

		for (int i = 0; i < BALL_COUNT; i++)
		{
			DirectX::CreateWICTextureFromFile(mDevice, L"Textures\\lava.png", nullptr, &mBallSRVs[i]);
		}
	}

	void InitializeCamera()
	{
		mCamera.mTransform.mPosition = { 0.0f, 0.0f, -30.0f };
	}
};

DECLARE_MAIN(BilliardsSample);