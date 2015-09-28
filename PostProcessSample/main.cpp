#include <Windows.h>
#include "Rig3D\Engine.h"
#include "Rig3D\Graphics\Interface\IScene.h"
#include "Rig3D\Graphics\DirectX11\DX3D11Renderer.h"
#include "Rig3D\Graphics\Interface\IMesh.h"
#include "Rig3D\Graphics\DirectX11\DX11Mesh.h"
#include "Rig3D\Common\Transform.h"
#include "Memory\Memory\LinearAllocator.h"
#include "Rig3D\Graphics\DirectX11\DirectXTK\Inc\WICTextureLoader.h"
#include <d3d11.h>
#include <d3dcompiler.h>

#define PI				3.1415926535f
#define CAMERA_SPEED	0.1f
#define RADIAN			3.1415926535f / 180.0f

using namespace Rig3D;

static const int VERTEX_COUNT			= 8;
static const int INDEX_COUNT			= 36;
static const float ANIMATION_DURATION	= 20000.0f; // 20 Seconds

class Rig3DSampleScene : public IScene, public virtual IRendererDelegate
{
public:

	typedef cliqCity::graphicsMath::Vector2 vec2f;
	typedef cliqCity::memory::LinearAllocator LinearAllocator;

	struct SampleVertex
	{
		vec3f mPosition;
		vec2f mUV;
	};

	struct SampleMatrixBuffer
	{
		mat4f mWorld;
		mat4f mView;
		mat4f mProjection;
	};

	struct BlurBuffer
	{
		vec2f uvOffsets[4];
		vec2f x[4];
	};
	
	struct SceneNode
	{
		Transform mTransform;
		IMesh*	  mMesh;
	};

	SceneNode				mFloorNode;
	Transform				mCamera;
	SampleMatrixBuffer		mMatrixBuffer;

	IMesh*					mCubeMesh;
	IMesh*					mQuadMesh;
	IMesh*					mFloorMesh;

	LinearAllocator			mAllocator;
	float					mMouseX;
	float					mMouseY;

	DX3D11Renderer*			mRenderer;
	ID3D11Device*			mDevice;
	ID3D11DeviceContext*	mDeviceContext;
	ID3D11Buffer*			mConstantBuffer;
	ID3D11InputLayout*		mInputLayout;
	ID3D11VertexShader*		mVertexShader;
	ID3D11PixelShader*		mPixelShader;

	// Use for blur
	ID3D11RenderTargetView*		mSceneRTV;
	ID3D11ShaderResourceView*	mSceneSRV;
	ID3D11Texture2D*			mSceneTexture2D;

	ID3D11RenderTargetView*		mBlurRTV;
	ID3D11ShaderResourceView*	mBlurSceneSRV;
	ID3D11Texture2D*			mBlurTexture2D;

	ID3D11ShaderResourceView*	mFloorSRV;
	ID3D11Texture2D*			mFloorTexture2D;

	ID3D11SamplerState*			mSamplerState;

	ID3D11VertexShader*			mQuadVertexShader;
	ID3D11PixelShader*			mQuadBlurPixelShader;

	BlurBuffer					mBlurH;
	BlurBuffer					mBlurV;
	ID3D11Buffer*				mBlurBuffer;

	vec2f					mPixelSize;
	float					mAnimationTime;
	short					mShouldPlay;

	Rig3DSampleScene() : mAllocator(1024)
	{
		mOptions.mWindowCaption	= "Rig3D Sample";
		mOptions.mWindowWidth	= 800;
		mOptions.mWindowHeight	= 600;
		mOptions.mGraphicsAPI   = GRAPHICS_API_DIRECTX11;
		mOptions.mFullScreen	= false;
		mAnimationTime	= 0.0f;
		mShouldPlay		= false;
	}

	~Rig3DSampleScene()
	{
		ReleaseMacro(mBlurRTV);
		ReleaseMacro(mBlurSceneSRV);
		ReleaseMacro(mBlurTexture2D);

		ReleaseMacro(mSceneRTV);
		ReleaseMacro(mSceneSRV);
		ReleaseMacro(mSceneTexture2D);

		ReleaseMacro(mVertexShader);
		ReleaseMacro(mPixelShader);
		ReleaseMacro(mQuadVertexShader);
		ReleaseMacro(mQuadVertexShader);

		ReleaseMacro(mConstantBuffer);
		ReleaseMacro(mInputLayout);

		ReleaseMacro(mBlurBuffer);
		ReleaseMacro(mSamplerState);
		ReleaseMacro(mQuadBlurPixelShader);

		ReleaseMacro(mFloorTexture2D);
		ReleaseMacro(mFloorSRV);
	}

	void VInitialize() override
	{
		mRenderer = &DX3D11Renderer::SharedInstance();
		mRenderer->SetDelegate(this);

		mDevice = mRenderer->GetDevice();
		mDeviceContext = mRenderer->GetDeviceContext(); 

		VOnResize();
		size_t s = 0;
		DirectX::CreateWICTextureFromFile(mDevice, L"Textures\\road.jpg", nullptr, &mFloorSRV);

		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		mDevice->CreateSamplerState(&samplerDesc, &mSamplerState);

		mMouseX = 0.0f;
		mMouseY = 0.0f;

		InitializeGeometry();
		InitializeShaders();
		InitializeCamera();
	}

	void InitializeGeometry()
	{
		SampleVertex vertices[VERTEX_COUNT];
		vertices[0].mPosition	= { -0.5f, +0.5f, +0.5f };	// Front Top Left
		vertices[0].mUV			= { +0.0f, +0.0f };

		vertices[1].mPosition	= { +0.5f, +0.5f, +0.5f };  // Front Top Right
		vertices[1].mUV			= { +1.0f, +0.0f};

		vertices[2].mPosition	= { +0.5f, -0.5f, +0.5f };  // Front Bottom Right
		vertices[2].mUV			= { +1.0f, +1.0f};

		vertices[3].mPosition	= { -0.5f, -0.5f, +0.5f };   // Front Bottom Left
		vertices[3].mUV			= { +0.0f, +1.0f};

		vertices[4].mPosition	= { -0.5f, +0.5f, -0.5f };;  // Back Top Left
		vertices[4].mUV			= { +1.0f, +0.0f};

		vertices[5].mPosition	= { +0.5f, +0.5f, -0.5f };  // Back Top Right
		vertices[5].mUV			= { +0.0f, +0.0f };

		vertices[6].mPosition	= { +0.5f, -0.5f, -0.5f };  // Back Bottom Right
		vertices[6].mUV			= { +0.0f, +1.0f };

		vertices[7].mPosition	= { -0.5f, -0.5f, -0.5f };  // Back Bottom Left
		vertices[7].mUV			= { +1.0f, +1.0f};

		uint16_t indices[INDEX_COUNT];
		// Front Face
		indices[0] = 0;
		indices[1] = 1;
		indices[2] = 2;

		indices[3] = 2;
		indices[4] = 3;
		indices[5] = 0;
		
		// Right Face
		indices[6] = 1;
		indices[7] = 5;
		indices[8] = 6;

		indices[9] = 6;
		indices[10] = 2;
		indices[11] = 1;

		// Back Face
		indices[12] = 5;
		indices[13] = 4;
		indices[14] = 7;

		indices[15] = 7;
		indices[16] = 6;
		indices[17] = 5;

		// Left Face
		indices[18] = 4;
		indices[19] = 0;
		indices[20] = 3;

		indices[21] = 3;
		indices[22] = 7;
		indices[23] = 4;

		// Top Face
		indices[24] = 4;
		indices[25] = 5;
		indices[26] = 1;

		indices[27] = 1;
		indices[28] = 0;
		indices[29] = 4;

		// Bottom Face
		indices[30] = 3;
		indices[31] = 2;
		indices[32] = 6;

		indices[33] = 6;
		indices[34] = 7;
		indices[35] = 3;

		mCubeMesh = new(mAllocator.Allocate(sizeof(DX11Mesh), alignof(DX11Mesh), 0)) DX11Mesh();
		mRenderer->VSetMeshVertexBufferData(mCubeMesh, vertices, sizeof(SampleVertex) * VERTEX_COUNT, sizeof(SampleVertex), GPU_MEMORY_USAGE_STATIC);
		mRenderer->VSetMeshIndexBufferData(mCubeMesh, indices, INDEX_COUNT, GPU_MEMORY_USAGE_STATIC);

		SampleVertex qVertices[4];
		qVertices[0].mPosition	= { -1.0f, 1.0f, 0.0f };
		qVertices[0].mUV		= { 0.0f, 0.0f};

		qVertices[1].mPosition	= { 1.0f, 1.0f, 0.0f };
		qVertices[1].mUV		= { 1.0f, 0.0f};

		qVertices[2].mPosition	= { 1.0f, -1.0f, 0.0f };
		qVertices[2].mUV		= { 1.0f, 1.0f };

		qVertices[3].mPosition	= { -1.0f, -1.0f, 0.0f };
		qVertices[3].mUV		= { 0.0f, 1.0f};

		uint16_t qIndices[6];
		qIndices[0] = 0;
		qIndices[1] = 1;
		qIndices[2] = 2;

		qIndices[3] = 2;
		qIndices[4] = 3;
		qIndices[5] = 0;

		mQuadMesh = new(mAllocator.Allocate(sizeof(DX11Mesh), alignof(DX11Mesh), 0)) DX11Mesh();
		mRenderer->VSetMeshVertexBufferData(mQuadMesh, qVertices, sizeof(SampleVertex) * 4, sizeof(SampleVertex), GPU_MEMORY_USAGE_STATIC);
		mRenderer->VSetMeshIndexBufferData(mQuadMesh, qIndices, 6, GPU_MEMORY_USAGE_STATIC);

		mFloorMesh = new(mAllocator.Allocate(sizeof(DX11Mesh), alignof(DX11Mesh), 0)) DX11Mesh();
		mRenderer->VSetMeshVertexBufferData(mFloorMesh, qVertices, sizeof(SampleVertex) * 4, sizeof(SampleVertex), GPU_MEMORY_USAGE_STATIC);
		mRenderer->VSetMeshIndexBufferData(mFloorMesh, qIndices, 6, GPU_MEMORY_USAGE_STATIC);

		//mFloorNode.mTransform.mRotation = { -0.5f * PI, 0.0f , 0.5f * PI };
		mFloorNode.mTransform.mScale = vec3f(5.0f);
		mFloorNode.mMesh = mFloorMesh;

		mCamera.mPosition = { 0.0f, 0.0, -10.0f };
}

	void InitializeShaders()
	{
		D3D11_INPUT_ELEMENT_DESC inputDescription[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		// Load Vertex Shader --------------------------------------
		ID3DBlob* vsBlob;
		D3DReadFileToBlob(L"VertexShader.cso", &vsBlob);

		// Create the shader on the device
		mDevice->CreateVertexShader(
			vsBlob->GetBufferPointer(),
			vsBlob->GetBufferSize(),
			NULL,
			&mVertexShader);

		// Before cleaning up the data, create the input layout
		if (inputDescription) {
			if (mInputLayout != NULL) ReleaseMacro(mInputLayout);
			mDevice->CreateInputLayout(
				inputDescription,					// Reference to Description
				2,									// Number of elments inside of Description
				vsBlob->GetBufferPointer(),
				vsBlob->GetBufferSize(),
				&mInputLayout);
		}

		// Clean up
		vsBlob->Release();

		// Load Pixel Shader ---------------------------------------
		ID3DBlob* psBlob;
		D3DReadFileToBlob(L"PixelShader.cso", &psBlob);

		// Create the shader on the device
		mDevice->CreatePixelShader(
			psBlob->GetBufferPointer(),
			psBlob->GetBufferSize(),
			NULL,
			&mPixelShader);

		// Clean up
		psBlob->Release();
	
		// Constant buffers ----------------------------------------
		D3D11_BUFFER_DESC cBufferTransformDesc;
		cBufferTransformDesc.ByteWidth = sizeof(mMatrixBuffer);
		cBufferTransformDesc.Usage = D3D11_USAGE_DEFAULT;
		cBufferTransformDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cBufferTransformDesc.CPUAccessFlags = 0;
		cBufferTransformDesc.MiscFlags = 0;
		cBufferTransformDesc.StructureByteStride = 0;

		mDevice->CreateBuffer(&cBufferTransformDesc, NULL, &mConstantBuffer);

		// Blur Shaders
		D3DReadFileToBlob(L"QuadVertexShader.cso", &vsBlob);

		mDevice->CreateVertexShader(
			vsBlob->GetBufferPointer(),
			vsBlob->GetBufferSize(),
			NULL,
			&mQuadVertexShader);

		if (inputDescription) {
			if (mInputLayout != NULL) ReleaseMacro(mInputLayout);
			mDevice->CreateInputLayout(
				inputDescription,					// Reference to Description
				2,									// Number of elments inside of Description
				vsBlob->GetBufferPointer(),
				vsBlob->GetBufferSize(),
				&mInputLayout);
		}

		vsBlob->Release();

		D3DReadFileToBlob(L"QuadPixelShader.cso", &psBlob);

		mDevice->CreatePixelShader(
			psBlob->GetBufferPointer(),
			psBlob->GetBufferSize(),
			NULL,
			&mQuadBlurPixelShader);

		psBlob->Release();

		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.ByteWidth = sizeof(BlurBuffer);
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		mDevice->CreateBuffer(&bufferDesc, NULL, &mBlurBuffer);
	}

	void InitializeCamera()
	{
		mMatrixBuffer.mProjection = mat4f::perspective(0.25f * PI, mRenderer->GetAspectRatio(), 0.1f, 100.0f).transpose();
		mMatrixBuffer.mView = mCamera.GetWorldMatrix().inverse().transpose();
	}

	void VUpdate(double milliseconds) override
	{
		InitializeCamera();

		mPixelSize = { 1.0f / mRenderer->GetWindowWidth(), 1.0f / mRenderer->GetWindowHeight() };
		
		mBlurH.uvOffsets[0] = { 0.0f, 0.0f };
		mBlurH.uvOffsets[1] = { -mPixelSize.x, 0.0f };
		mBlurH.uvOffsets[2] = { +mPixelSize.x, 0.0f };

		mBlurV.uvOffsets[0] = { 0.0f, 0.0f };
		mBlurV.uvOffsets[1] = { 0.0f, -mPixelSize.y };
		mBlurV.uvOffsets[2] = { 0.0f, +mPixelSize.y };

		if (Input::SharedInstance().GetKeyUp(KEYCODE_UP)) {
			mFloorNode.mTransform.RotatePitch(0.25f * PI);
		}

		if (Input::SharedInstance().GetKeyUp(KEYCODE_RIGHT)) {
			mFloorNode.mTransform.RotateRoll(0.25f * PI);
		}

		if (Input::SharedInstance().GetKeyUp(KEYCODE_DOWN)) {
			mFloorNode.mTransform.RotateYaw(0.25f * PI);
		}

		ScreenPoint mousePosition = Input::SharedInstance().mousePosition;
		if (Input::SharedInstance().GetMouseButton(MOUSEBUTTON_LEFT)) {
			mCamera.RotatePitch(-(mousePosition.y - mMouseY) * RADIAN);
			mCamera.RotateYaw(-(mousePosition.x - mMouseX) * RADIAN);	
		}

		mMouseX = mousePosition.x;
		mMouseY = mousePosition.y;

		if (Input::SharedInstance().GetKey(KEYCODE_W)) {
			mCamera.mPosition += mCamera.GetForward() * CAMERA_SPEED;
		}

		if (Input::SharedInstance().GetKey(KEYCODE_A)) {
			mCamera.mPosition += mCamera.GetRight() * -CAMERA_SPEED;
		}

		if (Input::SharedInstance().GetKey(KEYCODE_D)) {
			mCamera.mPosition += mCamera.GetRight() * CAMERA_SPEED;
		}

		if (Input::SharedInstance().GetKey(KEYCODE_S)) {
			mCamera.mPosition += mCamera.GetForward() * -CAMERA_SPEED;
		}
		
		mMatrixBuffer.mWorld = mFloorNode.mTransform.GetWorldMatrix().transpose();

		mAnimationTime += (float)milliseconds;
	}

	void VRender() override
	{
		float color[4] = { 0.5f, 1.0f, 1.0f, 1.0f };

		// Set up the input assembler
		mDeviceContext->IASetInputLayout(mInputLayout);
		mRenderer->VSetPrimitiveType(GPU_PRIMITIVE_TYPE_TRIANGLE);

		// Render scene to texture.
		mDeviceContext->RSSetViewports(1, &mRenderer->GetViewport());

		mDeviceContext->OMSetRenderTargets(1, &mSceneRTV, mRenderer->GetDepthStencilView());
		mDeviceContext->ClearRenderTargetView(mSceneRTV, color);
		mDeviceContext->ClearDepthStencilView(
			mRenderer->GetDepthStencilView(),
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			1.0f,
			0);

		mDeviceContext->VSSetShader(mVertexShader, NULL, 0);
		mDeviceContext->PSSetShader(mPixelShader, NULL, 0);

		mDeviceContext->UpdateSubresource(
			mConstantBuffer,
			0,
			NULL,
			&mMatrixBuffer,
			0,
			0);

		mDeviceContext->VSSetConstantBuffers(
			0,	
			1,
			&mConstantBuffer);

		mDeviceContext->PSSetShaderResources(0, 1, &mFloorSRV);
		mDeviceContext->PSSetSamplers(0, 1, &mSamplerState);

		mRenderer->VBindMesh(mFloorMesh);

		mRenderer->VDrawIndexed(0, mFloorMesh->GetIndexCount());

		// Horizontal pass gets rendered to the vertical
		mDeviceContext->OMSetRenderTargets(1, &mBlurRTV, mRenderer->GetDepthStencilView());
		mDeviceContext->ClearRenderTargetView(mBlurRTV, color);
		mDeviceContext->ClearDepthStencilView(
			mRenderer->GetDepthStencilView(),
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			1.0f,
			0);

		mDeviceContext->VSSetShader(mQuadVertexShader, NULL, 0);
		mDeviceContext->PSSetShader(mQuadBlurPixelShader, NULL, 0);

		// Bind mSceneTexture2D
		mDeviceContext->UpdateSubresource(
			mBlurBuffer,
			0,
			NULL,
			&mBlurH,
			0,
			0);

		mDeviceContext->PSSetConstantBuffers(
			0,
			1,
			&mBlurBuffer);

		mDeviceContext->PSSetShaderResources(0, 1, &mSceneSRV);
		mDeviceContext->PSSetSamplers(0, 1, &mSamplerState);

		mRenderer->VBindMesh(mQuadMesh);

		mRenderer->VDrawIndexed(0, mQuadMesh->GetIndexCount());

		// Final Pass
		mDeviceContext->RSSetViewports(1, &mRenderer->GetViewport());
		mDeviceContext->OMSetRenderTargets(1, mRenderer->GetRenderTargetView(), mRenderer->GetDepthStencilView());
		mDeviceContext->ClearRenderTargetView(*mRenderer->GetRenderTargetView(), color);
		mDeviceContext->ClearDepthStencilView(
			mRenderer->GetDepthStencilView(),
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			1.0f,
			0);

		// Bind Blur Texture
		mDeviceContext->PSSetShaderResources(0, 1, &mBlurSceneSRV);
		mDeviceContext->PSSetSamplers(0, 1, &mSamplerState);

		mDeviceContext->UpdateSubresource(
			mBlurBuffer,
			0,
			NULL,
			&mBlurV,
			0,
			0);

		mDeviceContext->PSSetConstantBuffers(
			0,
			1,
			&mBlurBuffer);

		mRenderer->VBindMesh(mQuadMesh);

		mRenderer->VDrawIndexed(0, mQuadMesh->GetIndexCount());

		ID3D11ShaderResourceView* nullSRV[1] = { 0 };
		mDeviceContext->PSSetShaderResources(0, 1, nullSRV);
		
		mRenderer->GetSwapChain()->Present(0, 0);
	}

	void VOnResize() override
	{
		ReleaseMacro(mBlurTexture2D);
		ReleaseMacro(mSceneTexture2D);
		ReleaseMacro(mBlurRTV);
		ReleaseMacro(mSceneRTV);
		ReleaseMacro(mBlurSceneSRV);

		D3D11_TEXTURE2D_DESC textureDesc;
		textureDesc.Width = mRenderer->GetWindowWidth();
		textureDesc.Height = mRenderer->GetWindowHeight();
		textureDesc.ArraySize = 1;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.MipLevels = 1;
		textureDesc.MiscFlags = 0;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;

		mDevice->CreateTexture2D(&textureDesc, 0, &mBlurTexture2D);
		mDevice->CreateTexture2D(&textureDesc, 0, &mSceneTexture2D);

		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		rtvDesc.Format = textureDesc.Format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;

		mRenderer->GetDevice()->CreateRenderTargetView(mBlurTexture2D, 0, &mBlurRTV);
		mRenderer->GetDevice()->CreateRenderTargetView(mSceneTexture2D, 0, &mSceneRTV);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;

		mDevice->CreateShaderResourceView(mBlurTexture2D, &srvDesc, &mBlurSceneSRV);
		mDevice->CreateShaderResourceView(mSceneTexture2D, &srvDesc, &mSceneSRV);
	}

	void VShutdown() override
	{
		mQuadMesh->~IMesh();
		mCubeMesh->~IMesh();
		mAllocator.Free();
	}
};

DECLARE_MAIN(Rig3DSampleScene);