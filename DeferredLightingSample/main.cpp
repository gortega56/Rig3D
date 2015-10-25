#include <Windows.h>
#include "Rig3D\Engine.h"
#include "Rig3D\Graphics\Interface\IScene.h"
#include "Rig3D\Graphics\DirectX11\DX3D11Renderer.h"
#include "Rig3D\Graphics\Interface\IMesh.h"
#include "Rig3D\Common\Transform.h"
#include "Memory\Memory\LinearAllocator.h"
#include "Rig3D\MeshLibrary.h"
#include "Rig3D/TaskDispatch/TaskDispatcher.h"
#include <d3d11.h>
#include <d3dcompiler.h>

#define PI						3.1415926535f
#define MULTITHREAD				4
#define THREAD_COUNT			1

using namespace Rig3D;

typedef cliqCity::memory::LinearAllocator LinearAllocator;

uint8_t gMemory[10240];
uint8_t gTaskMemory[1024];

struct Vertex2
{
	vec3f Position;
	vec2f UV;
};

struct Vertex4
{
	vec4f Tangent;
	vec3f Position;
	vec3f Normal;
	vec2f UV;
};

std::mutex gMemoryMutex;

void PerformModelLoadTask(const cliqCity::multicore::TaskData& data)
{
	OBJResource<Vertex4> resource(reinterpret_cast<const char*>(data.mKernelData));
	resource.mCalculateTangents = true;
	MeshLibrary<LinearAllocator>* meshLibrary = reinterpret_cast<MeshLibrary<LinearAllocator>*>(data.mStream.in[0]);
	IRenderer* drawContext = reinterpret_cast<IRenderer*>(data.mStream.in[1]);
	IMesh** mesh = reinterpret_cast<IMesh**>(data.mStream.in[2]);

	std::lock_guard<std::mutex> lock(gMemoryMutex);
	meshLibrary->LoadMesh(mesh, drawContext, resource);
}

class DeferredLightingScene : public IScene, public virtual IRendererDelegate
{
public:

	struct PointLight
	{
		vec4f Position;
	};

	struct SpotLight
	{
		vec3f Position;
		vec3f Direction;
		float Angle;
	};

	struct ModelViewProjection
	{
		mat4f Model;
		mat4f View;
		mat4f Projection;
	};

	struct SceneObject
	{
		Transform	mTransform;
		vec4f		mColor;
		IMesh*		mMesh;
	};

	ModelViewProjection				mMVP;
	SceneObject						mSceneObjects[5];

	MeshLibrary<LinearAllocator>	mMeshLibrary;
	LinearAllocator					mAllocator;
	
	DX3D11Renderer*					mRenderer;
	IMesh*							mTorusMesh;
	IMesh*							mSphereMesh;
	IMesh*							mConeMesh;
	IMesh*							mCubeMesh;
	IMesh*							mPlaneMesh;
	IMesh*							mQuadMesh;

	ID3D11DeviceContext*			mDeviceContext;
	ID3D11Device*					mDevice;

	ID3D11RenderTargetView*			mPositionRTV;
	ID3D11RenderTargetView*			mDepthRTV;
	ID3D11RenderTargetView*			mColorRTV;
	ID3D11RenderTargetView*			mNormalRTV;

	ID3D11Texture2D*				mPositionMap;
	ID3D11Texture2D*				mDepthMap;
	ID3D11Texture2D*				mColorMap;
	ID3D11Texture2D*				mNormalMap;

	ID3D11ShaderResourceView*		mPositionSRV;
	ID3D11ShaderResourceView*		mDepthSRV;
	ID3D11ShaderResourceView*		mColorSRV;
	ID3D11ShaderResourceView*		mNormalSRV;

	ID3D11SamplerState*				mSamplerState;

	ID3D11VertexShader*				mSceneVertexShader;
	ID3D11PixelShader*				mScenePixelShader;
	ID3D11VertexShader*				mQuadVertexShader;
	ID3D11PixelShader*				mQuadPixelShader;
	ID3D11PixelShader*				mSingleBufferPixelShader;

	ID3D11InputLayout*				mSceneInputLayout;
	ID3D11InputLayout*				mQuadInputLayout;

	ID3D11Buffer*					mMVPBuffer;
	ID3D11Buffer*					mColorBuffer;
	ID3D11Buffer*					mLightBuffer;

	D3D11_VIEWPORT					mPositionViewport;
	D3D11_VIEWPORT					mDepthViewport;
	D3D11_VIEWPORT					mColorViewport;
	D3D11_VIEWPORT					mNormalViewport;

	DeferredLightingScene() : 
		mAllocator(gMemory, gMemory + 10240),
		mRenderer(nullptr),
		mTorusMesh(nullptr),
		mSphereMesh(nullptr),
		mConeMesh(nullptr),
		mCubeMesh(nullptr),
		mPlaneMesh(nullptr),
		mQuadMesh(nullptr),
		mDeviceContext(nullptr),
		mDevice(nullptr),
		mPositionRTV(nullptr),
		mDepthRTV(nullptr),
		mColorRTV(nullptr),
		mNormalRTV(nullptr),
		mPositionMap(nullptr),
		mDepthMap(nullptr),
		mColorMap(nullptr),
		mNormalMap(nullptr),
		mPositionSRV(nullptr),
		mDepthSRV(nullptr),
		mColorSRV(nullptr),
		mNormalSRV(nullptr),
		mSamplerState(nullptr),
		mSceneVertexShader(nullptr),
		mScenePixelShader(nullptr),
		mQuadVertexShader(nullptr),
		mQuadPixelShader(nullptr),
		mSingleBufferPixelShader(nullptr),
		mSceneInputLayout(nullptr),
		mQuadInputLayout(nullptr),
		mMVPBuffer(nullptr),
		mColorBuffer(nullptr),
		mLightBuffer(nullptr)

	{
		mOptions.mWindowCaption = "Deferred Lighting Sample";
		mOptions.mWindowWidth = 800;
		mOptions.mWindowHeight = 600;
		mOptions.mGraphicsAPI = GRAPHICS_API_DIRECTX11;
		mOptions.mFullScreen = false;
		mMeshLibrary.SetAllocator(&mAllocator);
	};

	~DeferredLightingScene()
	{
		ReleaseMacro(mPositionRTV);
		ReleaseMacro(mDepthRTV);
		ReleaseMacro(mColorRTV);
		ReleaseMacro(mNormalRTV);
		ReleaseMacro(mPositionMap);
		ReleaseMacro(mDepthMap);
		ReleaseMacro(mColorMap);
		ReleaseMacro(mNormalMap);
		ReleaseMacro(mPositionSRV);
		ReleaseMacro(mDepthSRV);
		ReleaseMacro(mColorSRV);
		ReleaseMacro(mNormalSRV);

		ReleaseMacro(mSamplerState);

		ReleaseMacro(mSceneVertexShader);
		ReleaseMacro(mScenePixelShader);
		ReleaseMacro(mQuadVertexShader);
		ReleaseMacro(mQuadPixelShader);
		ReleaseMacro(mSingleBufferPixelShader);

		ReleaseMacro(mSceneInputLayout);
		ReleaseMacro(mQuadInputLayout);
	
		ReleaseMacro(mMVPBuffer);
		ReleaseMacro(mColorBuffer);
		ReleaseMacro(mLightBuffer);
	};

	void VInitialize() override
	{
		mRenderer = &DX3D11Renderer::SharedInstance();
		mRenderer->SetDelegate(this);
		mDevice = mRenderer->GetDevice();
		mDeviceContext = mRenderer->GetDeviceContext();

		InitializeGeometry();
		InitializeSceneObjects();
		InitializeShaders();
		InitializeShaderResources();
	}

	void VUpdate(double milliseconds) override
	{

	}

	void VRender() override
	{
		const float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		mRenderer->VSetPrimitiveType(GPU_PRIMITIVE_TYPE_TRIANGLE);
		mDeviceContext->RSSetViewports(1, &mRenderer->GetViewport());

		ID3D11RenderTargetView* RTVs[4] = { mPositionRTV, mDepthRTV, mColorRTV, mNormalRTV };
		mDeviceContext->OMSetRenderTargets(4, RTVs, mRenderer->GetDepthStencilView());
		mDeviceContext->ClearRenderTargetView(mPositionRTV, color);
		mDeviceContext->ClearRenderTargetView(mDepthRTV, color);
		mDeviceContext->ClearRenderTargetView(mColorRTV, color);
		mDeviceContext->ClearRenderTargetView(mNormalRTV, color);
		mDeviceContext->ClearDepthStencilView(
			mRenderer->GetDepthStencilView(),
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			1.0f,
			0);

		mDeviceContext->IASetInputLayout(mSceneInputLayout);
		mDeviceContext->VSSetShader(mSceneVertexShader, nullptr, 0);
		mDeviceContext->PSSetShader(mScenePixelShader, nullptr, 0);

		for (int i = 0; i < 5; i++)
		{
			mMVP.Model = mSceneObjects[i].mTransform.GetWorldMatrix().transpose();
			mDeviceContext->UpdateSubresource(mMVPBuffer, 0, nullptr, &mMVP, 0, 0);
			mDeviceContext->UpdateSubresource(mColorBuffer, 0, nullptr, &mSceneObjects[i].mColor, 0, 0);
			mDeviceContext->VSSetConstantBuffers(0, 1, &mMVPBuffer);
			mDeviceContext->PSSetConstantBuffers(0, 1, &mColorBuffer);
			mRenderer->VBindMesh(mSceneObjects[i].mMesh);
			mRenderer->VDrawIndexed(0, mSceneObjects[i].mMesh->GetIndexCount());
		}

		mDeviceContext->RSSetViewports(1, &mRenderer->GetViewport());
		mDeviceContext->OMSetRenderTargets(1, mRenderer->GetRenderTargetView(), mRenderer->GetDepthStencilView());
		mDeviceContext->ClearRenderTargetView(*mRenderer->GetRenderTargetView(), color);
		mDeviceContext->ClearDepthStencilView(
			mRenderer->GetDepthStencilView(),
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			1.0f,
			0);

		mDeviceContext->IASetInputLayout(mQuadInputLayout);
		mDeviceContext->VSSetShader(mQuadVertexShader, nullptr, 0);
		mDeviceContext->PSSetShader(mQuadPixelShader, nullptr, 0);

		ID3D11ShaderResourceView* SRVs[4] = { mPositionSRV, mDepthSRV, mColorSRV, mNormalSRV };
		mDeviceContext->PSSetShaderResources(0, 4, SRVs);
		mDeviceContext->PSSetSamplers(0, 1, &mSamplerState);

		mRenderer->VBindMesh(mQuadMesh);
		mRenderer->VDrawIndexed(0, mQuadMesh->GetIndexCount());

		D3D11_VIEWPORT viewports[] = { mPositionViewport, mDepthViewport, mColorViewport, mNormalViewport };
		mDeviceContext->VSSetShader(mQuadVertexShader, nullptr, 0);
		mDeviceContext->PSSetShader(mSingleBufferPixelShader, nullptr, 0);
		mDeviceContext->PSSetSamplers(0, 1, &mSamplerState);

		for (int i = 0; i < 4; i++) {
			mDeviceContext->RSSetViewports(1, &viewports[i]);
			mDeviceContext->ClearDepthStencilView(
				mRenderer->GetDepthStencilView(),
				D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
				1.0f,
				0);

			mDeviceContext->PSSetShaderResources(0, 1, &SRVs[i]);

			mRenderer->VBindMesh(mQuadMesh);
			mRenderer->VDrawIndexed(0, mQuadMesh->GetIndexCount());
		}

		ID3D11ShaderResourceView* nullSRV[4] = { 0, 0, 0, 0 };
		mDeviceContext->PSSetShaderResources(0, 4, nullSRV);

		mRenderer->VSwapBuffers();
	}

	void VShutdown() override
	{
		mTorusMesh->~IMesh();
		mSphereMesh->~IMesh();
		mConeMesh->~IMesh();
		mCubeMesh->~IMesh();
		mPlaneMesh->~IMesh();
		mQuadMesh->~IMesh();
		mAllocator.Free();
	}

	void VOnResize() override
	{
		ReleaseMacro(mPositionRTV);
		ReleaseMacro(mDepthRTV);
		ReleaseMacro(mColorRTV);
		ReleaseMacro(mNormalRTV);
		ReleaseMacro(mPositionMap);
		ReleaseMacro(mDepthMap);
		ReleaseMacro(mColorMap);
		ReleaseMacro(mNormalMap);
		ReleaseMacro(mPositionSRV);
		ReleaseMacro(mDepthSRV);
		ReleaseMacro(mColorSRV);
		ReleaseMacro(mNormalSRV);

		// Position
		{
			D3D11_TEXTURE2D_DESC positionTextureDesc;
			positionTextureDesc.Width = mRenderer->GetWindowWidth();
			positionTextureDesc.Height = mRenderer->GetWindowHeight();
			positionTextureDesc.ArraySize = 1;
			positionTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			positionTextureDesc.CPUAccessFlags = 0;
			positionTextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			positionTextureDesc.MipLevels = 1;
			positionTextureDesc.MiscFlags = 0;
			positionTextureDesc.SampleDesc.Count = 1;
			positionTextureDesc.SampleDesc.Quality = 0;
			positionTextureDesc.Usage = D3D11_USAGE_DEFAULT;

			mDevice->CreateTexture2D(&positionTextureDesc, nullptr, &mPositionMap);

			D3D11_RENDER_TARGET_VIEW_DESC positionRTVDesc;
			positionRTVDesc.Format = positionTextureDesc.Format;
			positionRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			positionRTVDesc.Texture2D.MipSlice = 0;

			mRenderer->GetDevice()->CreateRenderTargetView(mPositionMap, &positionRTVDesc, &mPositionRTV);

			D3D11_SHADER_RESOURCE_VIEW_DESC positionSRVDesc;
			positionSRVDesc.Format = positionTextureDesc.Format;
			positionSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			positionSRVDesc.Texture2D.MipLevels = 1;
			positionSRVDesc.Texture2D.MostDetailedMip = 0;

			mDevice->CreateShaderResourceView(mPositionMap, &positionSRVDesc, &mPositionSRV);
		}

		// Depth
		{	
			D3D11_TEXTURE2D_DESC depthTextureDesc;
			depthTextureDesc.Width = mRenderer->GetWindowWidth();
			depthTextureDesc.Height = mRenderer->GetWindowHeight();
			depthTextureDesc.ArraySize = 1;
			depthTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			depthTextureDesc.CPUAccessFlags = 0;
			depthTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			depthTextureDesc.MipLevels = 1;
			depthTextureDesc.MiscFlags = 0;
			depthTextureDesc.SampleDesc.Count = 1;
			depthTextureDesc.SampleDesc.Quality = 0;
			depthTextureDesc.Usage = D3D11_USAGE_DEFAULT;

			mDevice->CreateTexture2D(&depthTextureDesc, nullptr, &mDepthMap);

			D3D11_RENDER_TARGET_VIEW_DESC depthRTVDesc;
			depthRTVDesc.Format = depthTextureDesc.Format;
			depthRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			depthRTVDesc.Texture2D.MipSlice = 0;

			mRenderer->GetDevice()->CreateRenderTargetView(mDepthMap, &depthRTVDesc, &mDepthRTV);

			D3D11_SHADER_RESOURCE_VIEW_DESC depthSRVDesc;
			depthSRVDesc.Format = depthTextureDesc.Format;
			depthSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			depthSRVDesc.Texture2D.MipLevels = 1;
			depthSRVDesc.Texture2D.MostDetailedMip = 0;

			mDevice->CreateShaderResourceView(mDepthMap, &depthSRVDesc, &mDepthSRV);
		}

		// Color
		{	
			D3D11_TEXTURE2D_DESC colorTextureDesc;
			colorTextureDesc.Width = mRenderer->GetWindowWidth();
			colorTextureDesc.Height = mRenderer->GetWindowHeight();
			colorTextureDesc.ArraySize = 1;
			colorTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			colorTextureDesc.CPUAccessFlags = 0;
			colorTextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			colorTextureDesc.MipLevels = 1;
			colorTextureDesc.MiscFlags = 0;
			colorTextureDesc.SampleDesc.Count = 1;
			colorTextureDesc.SampleDesc.Quality = 0;
			colorTextureDesc.Usage = D3D11_USAGE_DEFAULT;

			mDevice->CreateTexture2D(&colorTextureDesc, nullptr, &mColorMap);

			D3D11_RENDER_TARGET_VIEW_DESC colorRTVDesc;
			colorRTVDesc.Format = colorTextureDesc.Format;
			colorRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			colorRTVDesc.Texture2D.MipSlice = 0;

			mRenderer->GetDevice()->CreateRenderTargetView(mColorMap, &colorRTVDesc, &mColorRTV);

			D3D11_SHADER_RESOURCE_VIEW_DESC colorSRVDesc;
			colorSRVDesc.Format = colorTextureDesc.Format;
			colorSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			colorSRVDesc.Texture2D.MipLevels = 1;
			colorSRVDesc.Texture2D.MostDetailedMip = 0;

			mDevice->CreateShaderResourceView(mColorMap, &colorSRVDesc, &mColorSRV);
		}

		// Normal
		{
			D3D11_TEXTURE2D_DESC normalTextureDesc;
			normalTextureDesc.Width = mRenderer->GetWindowWidth();
			normalTextureDesc.Height = mRenderer->GetWindowHeight();
			normalTextureDesc.ArraySize = 1;
			normalTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			normalTextureDesc.CPUAccessFlags = 0;
			normalTextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			normalTextureDesc.MipLevels = 1;
			normalTextureDesc.MiscFlags = 0;
			normalTextureDesc.SampleDesc.Count = 1;
			normalTextureDesc.SampleDesc.Quality = 0;
			normalTextureDesc.Usage = D3D11_USAGE_DEFAULT;

			mDevice->CreateTexture2D(&normalTextureDesc, nullptr, &mNormalMap);

			D3D11_RENDER_TARGET_VIEW_DESC normalRTVDesc;
			normalRTVDesc.Format = normalTextureDesc.Format;
			normalRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			normalRTVDesc.Texture2D.MipSlice = 0;

			mRenderer->GetDevice()->CreateRenderTargetView(mNormalMap, &normalRTVDesc, &mNormalRTV);

			D3D11_SHADER_RESOURCE_VIEW_DESC normalSRVDesc;
			normalSRVDesc.Format = normalTextureDesc.Format;
			normalSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			normalSRVDesc.Texture2D.MipLevels = 1;
			normalSRVDesc.Texture2D.MostDetailedMip = 0;

			mDevice->CreateShaderResourceView(mNormalMap, &normalSRVDesc, &mNormalSRV);
		}

		// Viewports
		{
			float width = static_cast<float>(mRenderer->GetWindowWidth()) * 0.25f;
			float windowHeight = static_cast<float>(mRenderer->GetWindowHeight());
			float height = windowHeight / 3.0f;
			float topY = windowHeight - height;

			mPositionViewport.TopLeftX = 0.0f;
			mPositionViewport.TopLeftY = topY;
			mPositionViewport.Width = width;
			mPositionViewport.Height = height;
			mPositionViewport.MinDepth = 0;
			mPositionViewport.MaxDepth = 1;

			mDepthViewport.TopLeftX = width;
			mDepthViewport.TopLeftY = topY;
			mDepthViewport.Width = width;
			mDepthViewport.Height = height;
			mDepthViewport.MinDepth = 0;
			mDepthViewport.MaxDepth = 1;

			mColorViewport.TopLeftX = width + width;
			mColorViewport.TopLeftY = topY;
			mColorViewport.Width = width;
			mColorViewport.Height = height;
			mColorViewport.MinDepth = 0;
			mColorViewport.MaxDepth = 1;

			mNormalViewport.TopLeftX = width + width + width;
			mNormalViewport.TopLeftY = topY;
			mNormalViewport.Width = width;
			mNormalViewport.Height = height;
			mNormalViewport.MinDepth = 0;
			mNormalViewport.MaxDepth = 1;
		}

		// Camera
		{
			mMVP.View = mat4f::lookAtLH(vec3f(0.0f, 0.0f, 0.0f), vec3f(0.0f, 0.0f, -10.0f), vec3f(0.0f, 1.0f, 0.0f)).transpose();
			mMVP.Projection = mat4f::normalizedPerspectiveLH(0.25f * PI, mRenderer->GetAspectRatio(), 0.1f, 100.0f).transpose();
		}
	}

	void InitializeGeometry()
	{
#ifdef MULTITHREAD
		cliqCity::multicore::Thread threads[THREAD_COUNT];
		cliqCity::multicore::TaskData modelData[4];
		char* fileNames[4] = {
			"Models\\torus.obj",
			"Models\\sphere.obj",
			"Models\\cone.obj",
			"Models\\cube.obj"
		};

		IMesh** meshes[4] = {
			&mTorusMesh,
			&mSphereMesh,
			&mConeMesh,
			&mCubeMesh
		};

		cliqCity::multicore::TaskID taskIDs[4];

		cliqCity::multicore::TaskDispatcher dispatchQueue(threads, THREAD_COUNT, gTaskMemory, 1024);
		dispatchQueue.Start();
		for (int i = 0; i < 4; i++)
		{
			modelData[i].mKernelData = fileNames[i];
			modelData[i].mStream.in[0] = &mMeshLibrary;
			modelData[i].mStream.in[1] = mRenderer;
			modelData[i].mStream.in[2] = meshes[i];
			taskIDs[i] = dispatchQueue.AddTask(modelData[i], PerformModelLoadTask);
		}

			//dispatchQueue.Synchronize();
#else
		OBJResource<Vertex4> torusResource("Models\\torus.obj");
		OBJResource<Vertex4> sphereResource("Models\\sphere.obj");
		OBJResource<Vertex4> coneResource("Models\\cone.obj");
		OBJResource<Vertex4> cubeResource("Models\\cube.obj");

		mMeshLibrary.LoadMesh(&mTorusMesh, mRenderer, torusResource);
		mMeshLibrary.LoadMesh(&mSphereMesh, mRenderer, sphereResource);
		mMeshLibrary.LoadMesh(&mConeMesh, mRenderer, coneResource);
		mMeshLibrary.LoadMesh(&mCubeMesh, mRenderer, cubeResource);
#endif
	
		Vertex4 planeVertices[9];
		for (int z = 0; z < 3; z++)
		{
			for (int x = 0; x < 3; x++)
			{
				Vertex4& vertex = planeVertices[z * 3 + x];
				vertex.Tangent = { 1.0f, 0.0f, 0.0f, 1.0f };
				vertex.Position = { -1.0f + x, 0.0f, 1.0f - z };
				vertex.Normal = { 0.0f, 1.0f, 0.0f };
				vertex.UV = { x / 3.0f, (1.0f + z / 3.0f) - 1.0f };
			}
		}

		uint16_t planeIndices[24] = { 0, 1, 4, 4, 3, 0, 1, 2, 5, 5, 4, 1, 3, 4, 7, 7, 6, 3, 4, 5, 8, 8, 7, 4 };

		mMeshLibrary.NewMesh(&mPlaneMesh, mRenderer);
		mRenderer->VSetMeshVertexBufferData(mPlaneMesh, planeVertices, sizeof(Vertex4) * 9, sizeof(Vertex4), GPU_MEMORY_USAGE_STATIC);
		mRenderer->VSetMeshIndexBufferData(mPlaneMesh, planeIndices, 24, GPU_MEMORY_USAGE_STATIC);

		Vertex2 quadVertices[4];
		quadVertices[0].Position = { -1.0f, 1.0f, 0.0f };
		quadVertices[0].UV = { 0.0f, 0.0f };
		quadVertices[1].Position = { 1.0f, 1.0f, 0.0f };
		quadVertices[1].UV = { 1.0f, 0.0f };
		quadVertices[2].Position = { 1.0f, -1.0f, 0.0f };
		quadVertices[2].UV = { 1.0f, 1.0f };
		quadVertices[3].Position = { -1.0f, -1.0f, 0.0f };
		quadVertices[3].UV = { 0.0f, 1.0f };

		uint16_t quadIndices[6] = { 0, 1, 2, 2, 3, 0 };

		mMeshLibrary.NewMesh(&mQuadMesh, mRenderer);
		mRenderer->VSetMeshVertexBufferData(mQuadMesh, quadVertices, sizeof(Vertex2) * 4, sizeof(Vertex2), GPU_MEMORY_USAGE_STATIC);
		mRenderer->VSetMeshIndexBufferData(mQuadMesh, quadIndices, 6, GPU_MEMORY_USAGE_STATIC);
	}

	void InitializeSceneObjects()
	{
		mSceneObjects[0].mTransform.mPosition = { -3.0f, 0.0f, 3.0f };
		mSceneObjects[0].mColor = { 1.0f, 1.0f, 0.0f, 1.0f };
		mSceneObjects[0].mMesh = mTorusMesh;

		mSceneObjects[1].mTransform.mPosition = { 2.5f, 0.0f, 2.5f };
		mSceneObjects[1].mColor = { 1.0f, 0.0f, 1.0f, 1.0f };
		mSceneObjects[1].mMesh = mSphereMesh;

		mSceneObjects[2].mTransform.mPosition = { 1.0f, 0.0f, 0.0f };
		mSceneObjects[2].mColor = { 0.0f, 1.0f, 1.0f, 1.0f };
		mSceneObjects[2].mMesh = mConeMesh;

		mSceneObjects[3].mTransform.mPosition = { -2.0f, 0.0f, -1.0f };
		mSceneObjects[3].mColor = { 1.0f, 0.0f, 0.0f, 1.0f };
		mSceneObjects[3].mMesh = mCubeMesh;

		mSceneObjects[4].mTransform.mPosition = { 0.0f, -1.0f, 0.0 };
		mSceneObjects[4].mColor = { 0.0f, 1.0f, 0.0f, 1.0f };
		mSceneObjects[4].mMesh = mPlaneMesh;
	}

	void InitializeShaders()
	{
		ID3DBlob* vsBlob;
		ID3DBlob* psBlob;

		D3D11_INPUT_ELEMENT_DESC vertex4InputDesc[] =
		{
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		D3DReadFileToBlob(L"ModelVertexShader.cso", &vsBlob);
		mDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &mSceneVertexShader);
		mDevice->CreateInputLayout(vertex4InputDesc, 4, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &mSceneInputLayout);
	
		D3DReadFileToBlob(L"ModelPixelShader.cso", &psBlob);
		mDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &mScenePixelShader);

		D3D11_INPUT_ELEMENT_DESC vertex2InputDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		D3DReadFileToBlob(L"QuadVertexShader.cso", &vsBlob);
		mDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &mQuadVertexShader);
		mDevice->CreateInputLayout(vertex2InputDesc, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &mQuadInputLayout);

		D3DReadFileToBlob(L"QuadPixelShader.cso", &psBlob);
		mDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &mQuadPixelShader);

		D3DReadFileToBlob(L"SingleBufferPixelShader.cso", &psBlob);
		mDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &mSingleBufferPixelShader);

		vsBlob->Release();
		psBlob->Release();
	}

	void InitializeShaderResources()
	{
		D3D11_BUFFER_DESC MVPBufferDesc;
		MVPBufferDesc.ByteWidth = sizeof(ModelViewProjection);
		MVPBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		MVPBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		MVPBufferDesc.CPUAccessFlags = 0;
		MVPBufferDesc.MiscFlags = 0;
		MVPBufferDesc.StructureByteStride = 0;

		mDevice->CreateBuffer(&MVPBufferDesc, nullptr, &mMVPBuffer);

		D3D11_BUFFER_DESC colorBufferDesc;
		colorBufferDesc.ByteWidth = sizeof(vec4f);
		colorBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		colorBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		colorBufferDesc.CPUAccessFlags = 0;
		colorBufferDesc.MiscFlags = 0;
		colorBufferDesc.StructureByteStride = 0;

		mDevice->CreateBuffer(&colorBufferDesc, nullptr, &mColorBuffer);

		D3D11_BUFFER_DESC lightBufferDesc;
		lightBufferDesc.ByteWidth = sizeof(PointLight);
		lightBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		lightBufferDesc.CPUAccessFlags = 0;
		lightBufferDesc.MiscFlags = 0;
		lightBufferDesc.StructureByteStride = 0;

		mDevice->CreateBuffer(&lightBufferDesc, nullptr, &mLightBuffer);

		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		mDevice->CreateSamplerState(&samplerDesc, &mSamplerState);

		VOnResize();
	}
};

DECLARE_MAIN(DeferredLightingScene);