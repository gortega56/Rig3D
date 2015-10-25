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

#define PI						3.1415926535f
#define BALL_COUNT				16
#define BALL_RADIUS				0.1077032961f
#define CAMERA_SPEED			0.1f
#define CAMERA_ROTATION_SPEED	0.1f
#define RADIAN					3.1415926535f / 180.0f

using namespace Rig3D;

static const int gMeshMemorySize = 2048;

char gMeshMemory[gMeshMemorySize];

class BilliardsSample : public IScene, public virtual IRendererDelegate
{
public:
	typedef cliqCity::memory::LinearAllocator LinearAllocator;

	struct Vertex3
	{
		vec3f Position;
		vec3f Normal;
		vec2f UV;
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

	struct ViewProjection
	{
		mat4f view;
		mat4f projection;
	};

	ViewProjection					mViewProjection;
	Transform						mBallTransforms[BALL_COUNT];
	mat4f							mBallWorldMatrices[BALL_COUNT];
	Camera							mCamera;

	LinearAllocator					mAllocator;
	MeshLibrary<LinearAllocator>	mMeshLibrary;

	float							mMouseX;
	float							mMouseY;

	DX3D11Renderer*					mRenderer;
	ID3D11DeviceContext*			mDeviceContext;
	ID3D11Device*					mDevice;

	PoolTable					mPoolTable;
	IMesh*						mBallMesh;

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

	ID3D11SamplerState*			mSamplerState;

	ID3D11Buffer*				mViewProjectionBuffer;
	ID3D11Buffer*				mBallTransformBuffer;

	BilliardsSample() :
		mAllocator(gMeshMemory, gMeshMemory + gMeshMemorySize),
		mMouseX(0.0f),
		mMouseY(0.0f),
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
		mPoolTableFeltSRV(nullptr),
		mSamplerState(nullptr),
		mViewProjectionBuffer(nullptr),
		mBallTransformBuffer(nullptr)
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
		ReleaseMacro(mSamplerState);
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
		VOnResize();
	}

	void InitializeGeometry()
	{
		OBJBasicResource<Vertex3> poolTableLegs("Models\\Legs.obj");
		//OBJResource<Vertex3> poolTableFeet("Models\\LegsMetalPart.obj");
		OBJBasicResource<Vertex3> poolTableSides("Models\\Side.obj");
		OBJBasicResource<Vertex3> poolTableGuards("Models\\SideGuard.obj");
		//OBJResource<Vertex3> poolTableSurface("Models\\TopSurface.obj");
		OBJBasicResource<Vertex3> poolTableBottom("Models\\Bottom.obj");
		//OBJResource<Vertex3> poolTableHoles("Models\\Holes.obj");
		OBJBasicResource<Vertex3> ball("Models\\Ball.obj");

		mMeshLibrary.LoadMesh(&mPoolTable.legs, mRenderer, poolTableLegs);
		//mMeshLibrary.LoadMesh(&mPoolTable.feet, mRenderer, poolTableFeet);
		mMeshLibrary.LoadMesh(&mPoolTable.sides, mRenderer, poolTableSides);
		mMeshLibrary.LoadMesh(&mPoolTable.guards, mRenderer, poolTableGuards);
		//mMeshLibrary.LoadMesh(&mPoolTable.surface, mRenderer, poolTableSurface);
		mMeshLibrary.LoadMesh(&mPoolTable.bottom, mRenderer, poolTableBottom);
		//mMeshLibrary.LoadMesh(&mPoolTable.holes, mRenderer, poolTableHoles);
		mMeshLibrary.LoadMesh(&mBallMesh, mRenderer, ball);

		mBallTransforms[0].mPosition = { 0.0f, 0.0f, -1.0f };

		float diameter = BALL_RADIUS * 2.0f;
		float xOffset = 0;
		float zOffset = 0;
		int xCount = 1;
		int zCount = 5;
		int i = 1;
		for (int z = 0; z < zCount; z++)
		{
			for (int x = 0; x < xCount; x++)
			{
				mBallTransforms[i].mPosition = { xOffset, 0.0f, zOffset };
				xOffset += diameter;
				i++;
			}

			xOffset = -(BALL_RADIUS * xCount * 0.5f);
			zOffset += diameter;
			xCount++;
		}
	}

	void InitializeShaders()
	{
		InitializeDefaultShaders();
		InitializeInstanceShaders();
	}

	void InitializeInstanceShaders()
	{
		D3D11_INPUT_ELEMENT_DESC inputDescription[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
		};

		ID3DBlob* vsBlob;
		D3DReadFileToBlob(L"BallVertexShader.cso", &vsBlob);

		mDevice->CreateVertexShader(
			vsBlob->GetBufferPointer(),
			vsBlob->GetBufferSize(),
			nullptr,
			&mBallVertexShader);

		if (inputDescription) {
			mDevice->CreateInputLayout(
				inputDescription,
				7,
				vsBlob->GetBufferPointer(),
				vsBlob->GetBufferSize(),
				&mBallInputLayout);
		}

		vsBlob->Release();

		ID3DBlob* psBlob;
		D3DReadFileToBlob(L"BallPixelShader.cso", &psBlob);

		mDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &mBallPixelShader);

		psBlob->Release();
	}

	void InitializeDefaultShaders()
	{
		D3D11_INPUT_ELEMENT_DESC inputDescription[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		ID3DBlob* vsBlob;
		D3DReadFileToBlob(L"DefaultVertexShader.cso", &vsBlob);

		mDevice->CreateVertexShader(
			vsBlob->GetBufferPointer(),
			vsBlob->GetBufferSize(),
			nullptr,
			&mPoolTableVertexShader);

		if (inputDescription) {
			mDevice->CreateInputLayout(
				inputDescription,
				3,
				vsBlob->GetBufferPointer(),
				vsBlob->GetBufferSize(),
				&mPoolTableInputLayout);
		}

		vsBlob->Release();

		ID3DBlob* psBlob;
		D3DReadFileToBlob(L"DefaultPixelShader.cso", &psBlob);

		mDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &mPoolTablePixelShader);

		psBlob->Release();
	}

	void InitializeShaderResources()
	{
		LoadBallTextures();
		LoadPoolTableTextures();
		InitializeSamplerState();
		InitializeBuffers();
	}

	void LoadBallTextures()
	{
		const wchar_t* filenames[BALL_COUNT] = {
			L"Textures\\0.png",
			L"Textures\\1.png",
			L"Textures\\2.png",
			L"Textures\\3.png",
			L"Textures\\4.png",
			L"Textures\\5.png",
			L"Textures\\6.png",
			L"Textures\\7.png",
			L"Textures\\8.png",
			L"Textures\\10.png",
			L"Textures\\10.png",
			L"Textures\\11.png",
			L"Textures\\12.png",
			L"Textures\\13.png",
			L"Textures\\14.png",
			L"Textures\\15.png"
		};

		ID3D11Texture2D* ballTextures[BALL_COUNT];

		for (int i = 0; i < BALL_COUNT; i++)
		{
			DirectX::CreateWICTextureFromFileEx(mDevice, filenames[i], 0, D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_READ, 0, false, reinterpret_cast<ID3D11Resource**>(&ballTextures[i]), nullptr);
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
		mDevice->CreateTexture2D(&textureArrayDesc, nullptr, &textureArray);

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
			ReleaseMacro(ballTextures[i]);
		}
	}

	void LoadPoolTableTextures()
	{
		DirectX::CreateWICTextureFromFile(mDevice, L"Textures\\cherrywood.png", nullptr, &mPoolTableWoodSRV);
		DirectX::CreateWICTextureFromFile(mDevice, L"Textures\\chocolatewood.png", nullptr, &mPoolTableDarkWoodSRV);
		DirectX::CreateWICTextureFromFile(mDevice, L"Textures\\concrete.jpg", nullptr, &mPoolTableConcreteSRV);
		DirectX::CreateWICTextureFromFile(mDevice, L"Textures\\felt.jpg", nullptr, &mPoolTableFeltSRV);
	}

	void InitializeSamplerState()
	{
		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		mDevice->CreateSamplerState(&samplerDesc, &mSamplerState);
	}

	void InitializeBuffers()
	{
		D3D11_BUFFER_DESC ballTransformBufferDesc;
		ballTransformBufferDesc.ByteWidth = sizeof(mat4f) * BALL_COUNT;
		ballTransformBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		ballTransformBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		ballTransformBufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		ballTransformBufferDesc.MiscFlags = 0;
		ballTransformBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA transformData;
		transformData.pSysMem = mBallWorldMatrices;
		mDevice->CreateBuffer(&ballTransformBufferDesc, &transformData, &mBallTransformBuffer);

		D3D11_BUFFER_DESC viewProjectionBufferDesc;
		viewProjectionBufferDesc.ByteWidth = sizeof(ViewProjection);
		viewProjectionBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		viewProjectionBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		viewProjectionBufferDesc.CPUAccessFlags = 0;
		viewProjectionBufferDesc.MiscFlags = 0;
		viewProjectionBufferDesc.StructureByteStride = 0;

		mDevice->CreateBuffer(&viewProjectionBufferDesc, nullptr, &mViewProjectionBuffer);
	}

	void InitializeCamera()
	{
		mCamera.mTransform.mPosition = { 0.0f, 0.0f, -10.0f };
	}

	void VUpdate(double milliseconds) override
	{

		UpdateCamera();
		UpdateBallTransforms();
	}

	void UpdateCamera()
	{
		ScreenPoint mousePosition = Input::SharedInstance().mousePosition;
		if (Input::SharedInstance().GetMouseButton(MOUSEBUTTON_LEFT)) {
			mCamera.mTransform.RotatePitch(-(mousePosition.y - mMouseY) * RADIAN * CAMERA_ROTATION_SPEED);
			mCamera.mTransform.RotateYaw(-(mousePosition.x - mMouseX) * RADIAN * CAMERA_ROTATION_SPEED);
		}

		mMouseX = mousePosition.x;
		mMouseY = mousePosition.y;

		if (Input::SharedInstance().GetKey(KEYCODE_W)) {
			mCamera.mTransform.mPosition += mCamera.mTransform.GetForward() * CAMERA_SPEED;
		}

		if (Input::SharedInstance().GetKey(KEYCODE_A)) {
			mCamera.mTransform.mPosition += mCamera.mTransform.GetRight() * -CAMERA_SPEED;
		}

		if (Input::SharedInstance().GetKey(KEYCODE_D)) {
			mCamera.mTransform.mPosition += mCamera.mTransform.GetRight() * CAMERA_SPEED;
		}

		if (Input::SharedInstance().GetKey(KEYCODE_S)) {
			mCamera.mTransform.mPosition += mCamera.mTransform.GetForward() * -CAMERA_SPEED;
		}

		mViewProjection.view = mat4f::lookAtLH(mCamera.mTransform.mPosition + mCamera.mTransform.GetForward(), mCamera.mTransform.mPosition, vec3f(0.0f, 1.0f, 0.0f)).transpose();
	}

	void UpdateBallTransforms()
	{
		for (int i = 0; i < BALL_COUNT; i++)
		{
			mBallWorldMatrices[i] = mBallTransforms[i].GetWorldMatrix().transpose();
		}

		D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		mDeviceContext->Map(mBallTransformBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
		memcpy(mappedSubresource.pData, mBallWorldMatrices, sizeof(mat4f) * BALL_COUNT);
		mDeviceContext->Unmap(mBallTransformBuffer, 0);
	}

	void VRender() override
	{
		float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

		mDeviceContext->RSSetViewports(1, &mRenderer->GetViewport());
		mDeviceContext->OMSetRenderTargets(1, mRenderer->GetRenderTargetView(), mRenderer->GetDepthStencilView());
		mDeviceContext->ClearRenderTargetView(*mRenderer->GetRenderTargetView(), color);
		mDeviceContext->ClearDepthStencilView(
			mRenderer->GetDepthStencilView(),
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			1.0f,
			0);

		mRenderer->VSetPrimitiveType(GPU_PRIMITIVE_TYPE_TRIANGLE);
		mDeviceContext->UpdateSubresource(mViewProjectionBuffer, 0, nullptr, &mViewProjection, 0, 0);

		RenderBalls();
		RenderPoolTable();

		mRenderer->VSwapBuffers();
	}

	void RenderBalls()
	{
		mDeviceContext->IASetInputLayout(mBallInputLayout);
		mDeviceContext->VSSetShader(mBallVertexShader, nullptr, 0);
		mDeviceContext->PSSetShader(mBallPixelShader, nullptr, 0);

		mDeviceContext->VSSetConstantBuffers(0, 1, &mViewProjectionBuffer);
		mDeviceContext->PSSetShaderResources(0, 1, &mBallSRV);
		mDeviceContext->PSSetSamplers(0, 1, &mSamplerState);

		mRenderer->VBindMesh(mBallMesh);

		const UINT stride = sizeof(mat4f);
		const UINT offset = 0;
		mDeviceContext->IASetVertexBuffers(1, 1, &mBallTransformBuffer, &stride, &offset);

		mDeviceContext->DrawIndexedInstanced(mBallMesh->GetIndexCount(), BALL_COUNT, 0, 0, 0);
	}

	void RenderPoolTable()
	{
		mDeviceContext->IASetInputLayout(mPoolTableInputLayout);
		mDeviceContext->VSSetShader(mPoolTableVertexShader, nullptr, 0);
		mDeviceContext->PSSetShader(mPoolTablePixelShader, nullptr, 0);

		mDeviceContext->VSSetConstantBuffers(0, 1, &mViewProjectionBuffer);
		mDeviceContext->PSSetSamplers(0, 1, &mSamplerState);

		const UINT stride = sizeof(Vertex3);
		const UINT offset = 0;

		// Legs
		mDeviceContext->PSSetShaderResources(0, 1, &mPoolTableWoodSRV);
		mRenderer->VBindMesh(mPoolTable.legs);
		mRenderer->VDrawIndexed(0, mPoolTable.legs->GetIndexCount());

		// Feet
		mDeviceContext->PSSetShaderResources(0, 1, &mPoolTableConcreteSRV);
	//	mRenderer->VBindMesh(mPoolTable.feet);
	//	mRenderer->VDrawIndexed(0, mPoolTable.feet->GetIndexCount());

		// Guards
		mRenderer->VBindMesh(mPoolTable.guards);
		mRenderer->VDrawIndexed(0, mPoolTable.guards->GetIndexCount());

		// Surface
	//	mDeviceContext->PSSetShaderResources(0, 1, &mPoolTableFeltSRV);
	//	mRenderer->VBindMesh(mPoolTable.surface);
	//	mRenderer->VDrawIndexed(0, mPoolTable.surface->GetIndexCount());


		// Bottom 
		mDeviceContext->PSSetShaderResources(0, 1, &mPoolTableDarkWoodSRV);
		mRenderer->VBindMesh(mPoolTable.bottom);
		mRenderer->VDrawIndexed(0, mPoolTable.bottom->GetIndexCount());

		// Holes
		//mRenderer->VBindMesh(mPoolTable.holes);
		//mRenderer->VDrawIndexed(0, mPoolTable.holes->GetIndexCount());

		// Sides
		mRenderer->VBindMesh(mPoolTable.sides);
		mRenderer->VDrawIndexed(0, mPoolTable.sides->GetIndexCount());
	}

	void VShutdown() override
	{

	}

	void VOnResize() override
	{
		mViewProjection.projection = mat4f::normalizedPerspectiveLH(PI * 0.25f, mRenderer->GetAspectRatio(), 0.1f, 100.0f).transpose();
	}
};

DECLARE_MAIN(BilliardsSample);