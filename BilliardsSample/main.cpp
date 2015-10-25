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

	ID3D11InputLayout*			mBallInputLayout;
	ID3D11VertexShader*			mBallVertexShader;
	ID3D11PixelShader*			mBallPixelShader;
	ID3D11ShaderResourceView*	mBallSRV;

	ID3D11InputLayout*			mPoolTableInputLayout;
	ID3D11VertexShader*			mPoolTableVertexShader;
	ID3D11PixelShader*			mPoolTablePixelShader;
	ID3D11ShaderResourceView*   mPoolTableWoodSRV;
	ID3D11ShaderResourceView*   mPoolTableDarkWoodSRV;
	ID3D11ShaderResourceView*	mPoolTableConcreteSRV;
	ID3D11ShaderResourceView*	mPoolTableFeltSRV;

	BilliardsSample() : 
		mAllocator(gMeshMemory, gMeshMemory + gMeshMemorySize),
		mRenderer(nullptr),
		mDeviceContext(nullptr),
		mDevice(nullptr),
		mBallMesh(nullptr), 
		mBallInputLayout(nullptr),
		mBallVertexShader(nullptr),
		mBallPixelShader(nullptr),
		mBallSRV(nullptr),
		mPoolTableInputLayout(nullptr),
		mPoolTableVertexShader(nullptr),
		mPoolTablePixelShader(nullptr),
		mPoolTableWoodSRV(nullptr),
		mPoolTableDarkWoodSRV(nullptr),
		mPoolTableConcreteSRV(nullptr),
		mPoolTableFeltSRV(nullptr)
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
		ReleaseMacro(mBallInputLayout);
		ReleaseMacro(mBallVertexShader);
		ReleaseMacro(mBallPixelShader);
		ReleaseMacro(mBallSRV);

		ReleaseMacro(mPoolTableInputLayout);
		ReleaseMacro(mPoolTableVertexShader);
		ReleaseMacro(mPoolTablePixelShader);
		ReleaseMacro(mPoolTableWoodSRV);
		ReleaseMacro(mPoolTableDarkWoodSRV);
		ReleaseMacro(mPoolTableConcreteSRV);
		ReleaseMacro(mPoolTableFeltSRV);
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
		LoadBallTextures();
	}

	void LoadBallTextures()
	{
		ID3D11ShaderResourceView*	ballSRVs[BALL_COUNT];
		ID3D11Texture2D*			ballTextures[BALL_COUNT];

		const wchar_t* filenames[BALL_COUNT] = {
			L"Textures\\pallina0.jpg",
			L"Textures\\pallina1.jpg",
			L"Textures\\pallina2.jpg",
			L"Textures\\pallina3.jpg",
			L"Textures\\pallina4.jpg",
			L"Textures\\pallina5.jpg",
			L"Textures\\pallina6.jpg",
			L"Textures\\pallina7.jpg",
			L"Textures\\pallina8.jpg",
			L"Textures\\pallina9.jpg",
			L"Textures\\pallina10.jpg",
			L"Textures\\pallina11.jpg",
			L"Textures\\pallina12.jpg",
			L"Textures\\pallina13.jpg",
			L"Textures\\pallina14.jpg",
			L"Textures\\pallina15.jpg"
		};

		for (int i = 0; i < BALL_COUNT; i++)
		{
			DirectX::CreateWICTextureFromFile(mDevice, filenames[i], reinterpret_cast<ID3D11Resource**>(&ballTextures[i]), &ballSRVs[i]);
		}

		D3D11_TEXTURE2D_DESC textureDesc;
		ballTextures[0]->GetDesc(&textureDesc);

		D3D11_TEXTURE2D_DESC textureArrayDesc;
		textureArrayDesc.Width = textureDesc.Width;
		textureArrayDesc.Height = textureDesc.Height;
		textureArrayDesc.MipLevels = textureDesc.MipLevels;
		textureArrayDesc.ArraySize = BALL_COUNT;
		textureArrayDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureArrayDesc.SampleDesc.Count = 1;
		textureArrayDesc.SampleDesc.Quality = 0;
		textureArrayDesc.Usage = D3D11_USAGE_DEFAULT;
		textureArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureArrayDesc.CPUAccessFlags = 0;
		textureArrayDesc.MiscFlags = 0;

		ID3D11Texture2D* textureArray;
		mDevice->CreateTexture2D(&textureArrayDesc, 0, &textureArray);

		D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		for (int i = 0; i < BALL_COUNT; i++)
		{
			for (int j = 0; j < textureDesc.MipLevels; j++)
			{
				mDeviceContext->Map(ballTextures[i], j, D3D11_MAP_READ, 0, &mappedSubresource);
				mDeviceContext->UpdateSubresource(textureArray, D3D11CalcSubresource(j, i, textureDesc.MipLevels), nullptr, mappedSubresource.pData, mappedSubresource.RowPitch, 0);
				mDeviceContext->Unmap(ballTextures[i], j);
			}
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
		SRVDesc.Format = textureArrayDesc.Format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		SRVDesc.Texture2DArray.MostDetailedMip = 0;
		SRVDesc.Texture2DArray.MipLevels = textureArrayDesc.MipLevels;
		SRVDesc.Texture2DArray.FirstArraySlice = 0;
		SRVDesc.Texture2DArray.ArraySize = BALL_COUNT;

		mDevice->CreateShaderResourceView(textureArray, &SRVDesc, &mBallSRV);

		ReleaseMacro(textureArray);
		for (int i = 0; i < BALL_COUNT; i++)
		{
			ReleaseMacro(ballSRVs[i]);
			ReleaseMacro(ballTextures[i]);
		}
	}

	void LoadPoolTableTextures()
	{
		DirectX::CreateWICTextureFromFile(mDevice, L"", nullptr, &mPoolTableWoodSRV);
		DirectX::CreateWICTextureFromFile(mDevice, L"", nullptr, &mPoolTableDarkWoodSRV);
		DirectX::CreateWICTextureFromFile(mDevice, L"", nullptr, &mPoolTableConcreteSRV);
		DirectX::CreateWICTextureFromFile(mDevice, L"", nullptr, &mPoolTableFeltSRV);
	}

	void InitializeCamera()
	{
		mCamera.mTransform.mPosition = { 0.0f, 0.0f, -30.0f };
	}
};

DECLARE_MAIN(BilliardsSample);