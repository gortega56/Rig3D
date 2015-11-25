#include <Windows.h>
#include "Rig3D\Engine.h"
#include "Rig3D\Graphics\Interface\IScene.h"
#include "Rig3D\Graphics\DirectX11\DX3D11Renderer.h"
#include "Rig3D\Graphics\Interface\IRenderContext.h"
#include "Rig3D\Graphics\Interface\IMesh.h"
#include "Rig3D\Graphics\Interface\IShader.h"
#include "Rig3D\Graphics\Interface\IShaderResource.h"
#include "Rig3D\Common\Transform.h"
#include "Memory\Memory\Memory.h"
#include "Rig3D\Graphics\MeshLibrary.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <random>
#include <ctime>

#define SATURATE_RANDOM			FLT_EPSILON + (float)(rand()) / ((float)(RAND_MAX / (1.0f - FLT_EPSILON)))
#define OFFSET_COUNT			12
#define PI						3.1415926535f
#define CAMERA_SPEED			0.1f
#define CAMERA_ROTATION_SPEED	0.1f
#define RADIAN					3.1415926535f / 180.0f
#define NODE_COUNT				8

using namespace Rig3D;

static const int VERTEX_COUNT			= 8;
static const int INDEX_COUNT			= 36;
static const float ANIMATION_DURATION	= 20000.0f; // 20 Seconds

class Rig3DSampleScene : public IScene, public virtual IRendererDelegate
{
public:
	enum BlurType
	{
		BLUR_TYPE_NONE,
		BLUR_TYPE_GAUSSIAN,
		BLUR_TYPE_MOTION
	};

	struct SampleVertex
	{
		vec3f mPosition;
		vec2f mUV;
	};

	struct Vertex4 
	{
		vec4f Tangent;
		vec3f Position;
		vec3f Normal;
		vec2f UV;
	};

	struct SampleMatrixBuffer
	{
		mat4f mWorld;
		mat4f mView;
		mat4f mProjection;
	};

	struct BlurBuffer
	{
		vec2f uvOffsets[OFFSET_COUNT];
	};

	struct MBMatrixBuffer
	{
		mat4f mInverseClip;
		mat4f mPreviousClip;
	};
	
	struct SceneNode
	{
		Transform mTransform;
		vec4f	  mColor;
		IMesh*	  mMesh;
	};

	SampleMatrixBuffer		mMatrixBuffer;
	MBMatrixBuffer			mMBMatrixBuffer;
	BlurBuffer				mBlurH;
	BlurBuffer				mBlurV;
	SceneNode				mSceneNodes[NODE_COUNT];
	Transform				mCamera;

	vec2f					mPixelSize;
	vec4f					mClearColor;
	float					mMouseX;
	float					mMouseY;
	BlurType				mBlurType;

	LinearAllocator			mAllocator;

	MeshLibrary<LinearAllocator>	mMeshLibrary;
	IRenderer*						mRenderer;
	IMesh*							mCubeMesh;
	IMesh*							mQuadMesh;

	IRenderContext*					mRenderContext;
	IShaderResource*				mBlurShaderResource;
	IShaderResource*				mSphereShaderResource;

	IShader*				mVertexShader;
	IShader*				mPixelShader;
	IShader*				mSCPixelShader;
	IShader*				mQuadVertexShader;
	IShader*				mQuadBlurPixelShader;
	IShader*				mMotionBlurPixelShader;

	ID3D11SamplerState*		mSamplerState;

	Rig3DSampleScene() : 
		mMouseX(0.0f),
		mMouseY(0.0f),
		mAllocator(2048), 
		mRenderer(nullptr),
		mCubeMesh(nullptr), 
		mQuadMesh(nullptr), 
		mRenderContext(nullptr),
		mBlurShaderResource(nullptr),
		mSphereShaderResource(nullptr),
		mVertexShader(nullptr),
		mPixelShader(nullptr),
		mSCPixelShader(nullptr),
		mQuadVertexShader(nullptr),
		mQuadBlurPixelShader(nullptr),
		mMotionBlurPixelShader(nullptr),
		mSamplerState(nullptr)
	{
		mOptions.mWindowCaption	= "Rig3D Sample";
		mOptions.mWindowWidth	= 800;
		mOptions.mWindowHeight	= 600;
		mOptions.mGraphicsAPI   = GRAPHICS_API_DIRECTX11;
		mOptions.mFullScreen	= false;
		mBlurType				= BLUR_TYPE_NONE;
		mClearColor				= { 0.2f, 0.2f, 0.2f, 1.0f };
		mMeshLibrary.SetAllocator(&mAllocator);
	}

	~Rig3DSampleScene()
	{
		ReleaseMacro(mSamplerState);
	}

	void VInitialize() override
	{
		mRenderer = &DX3D11Renderer::SharedInstance();
		mRenderer->SetDelegate(this);

		mRenderer->VCreateRenderContext(&mRenderContext, &mAllocator);

		VOnResize();

		mMouseX = 0.0f;
		mMouseY = 0.0f;

		std::time_t now;
		std::srand(static_cast<unsigned int>(std::time(&now)));

		InitializeGeometry();
		InitializeShaders();
		InitializeCamera();
	}

	void InitializeGeometry()
	{
		OBJResource<Vertex4> resource ("Models\\Sphere.obj");
		mMeshLibrary.LoadMesh(&mCubeMesh, mRenderer, resource);

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

		mMeshLibrary.NewMesh(&mQuadMesh, mRenderer);
		mRenderer->VSetStaticMeshVertexBuffer(mQuadMesh, qVertices, sizeof(SampleVertex) * 4, sizeof(SampleVertex));
		mRenderer->VSetStaticMeshIndexBuffer(mQuadMesh, qIndices, 6);

		for (int i = 0; i < NODE_COUNT; i++) {
			mSceneNodes[i].mMesh = mCubeMesh;
			mSceneNodes[i].mColor = { SATURATE_RANDOM, SATURATE_RANDOM, SATURATE_RANDOM, 1.0f };
			mSceneNodes[i].mTransform.SetPosition((float)(rand() % 10) - 5.0f, (float)(rand() % 10) - 5.0f, (float)(rand() % 5));
		}

		mCamera.SetPosition( 0.0f, 0.0, -10.0f );
}

	void InitializeShaders()
	{
		// Sphere Shaders
		{
			InputElement sphereInputElements[] =
			{
				{ "TANGENT",	0, 0, 0, 0, RGBA_FLOAT32, INPUT_CLASS_PER_VERTEX },
				{ "POSITION",	0, 0, 16, 0, RGB_FLOAT32, INPUT_CLASS_PER_VERTEX },
				{ "NORMAL",		0, 0, 28, 0, RGB_FLOAT32, INPUT_CLASS_PER_VERTEX },
				{ "TEXCOORD",	0, 0, 40, 0, RG_FLOAT32, INPUT_CLASS_PER_VERTEX }
			};

			mRenderer->VCreateShader(&mVertexShader, &mAllocator);
			mRenderer->VLoadVertexShader(mVertexShader, "SphereVertexShader.cso", sphereInputElements, 4);

			mRenderer->VCreateShader(&mPixelShader, &mAllocator);
			mRenderer->VLoadPixelShader(mPixelShader, "SpherePixelShader.cso");

			mRenderer->VCreateShader(&mSCPixelShader, &mAllocator);
			mRenderer->VLoadPixelShader(mSCPixelShader, "SCPixelShader.cso");

			mRenderer->VCreateShaderResource(&mSphereShaderResource, &mAllocator);

			void* data[] = { &mMatrixBuffer, nullptr };
			size_t sizes[] = { sizeof(SampleMatrixBuffer), sizeof(vec4f) };
			mRenderer->VCreateShaderConstantBuffers(mSphereShaderResource, data, sizes, 2);

			const char* filenames[] =
			{
				"Textures\\lava.png",
				"Textures\\water.png"
			};

			mRenderer->VCreateShaderTextures2D(mSphereShaderResource, filenames, 2);

			//D3D11_SAMPLER_DESC samplerDesc;
			//ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
			//samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			//samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			//samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			//samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			//samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

			//ID3D11Device* device = static_cast<DX3D11Renderer*>(mRenderer)->GetDevice();
			//device->CreateSamplerState(&samplerDesc, &mSamplerState);
		}

		// Blur shaders
		{
			InputElement blurInputElement[] =
			{
				{ "POSITION",	0, 0, 0, 0, RGB_FLOAT32, INPUT_CLASS_PER_VERTEX },
				{ "TEXCOORD",	0, 0, 12, 0, RG_FLOAT32, INPUT_CLASS_PER_VERTEX }
			};

			mRenderer->VCreateShader(&mQuadVertexShader, &mAllocator);
			mRenderer->VLoadVertexShader(mQuadVertexShader, "BlurVertexShader.cso", blurInputElement, 2);

			mRenderer->VCreateShader(&mQuadBlurPixelShader, &mAllocator);
			mRenderer->VLoadPixelShader(mQuadBlurPixelShader, "BlurPixelShader.cso");

			mRenderer->VCreateShader(&mMotionBlurPixelShader, &mAllocator);
			mRenderer->VLoadPixelShader(mMotionBlurPixelShader, "MBQPixelShader.cso");

			mRenderer->VCreateShaderResource(&mBlurShaderResource, &mAllocator);

			void* data[] = { &mMBMatrixBuffer, &mBlurH };
			size_t sizes[] = { sizeof(MBMatrixBuffer), sizeof(BlurBuffer) };
			mRenderer->VCreateShaderConstantBuffers(mBlurShaderResource, data, sizes, 2);

			mRenderer->VAddShaderLinearSamplerState(mBlurShaderResource, SAMPLER_STATE_ADDRESS_CLAMP);
		}
	}

	void InitializeCamera()
	{
		mMBMatrixBuffer.mPreviousClip = mMatrixBuffer.mProjection * mMatrixBuffer.mView * mat4f(1.0f);
		mMatrixBuffer.mProjection = mat4f::normalizedPerspectiveLH(0.25f * PI, mRenderer->GetAspectRatio(), 0.1f, 100.0f).transpose();
		mMatrixBuffer.mView = mCamera.GetWorldMatrix().inverse().transpose();
		mMBMatrixBuffer.mInverseClip = (mMatrixBuffer.mProjection * mMatrixBuffer.mView * mat4f(1.0f)).inverse();
		mat4f i = mMBMatrixBuffer.mPreviousClip * mMBMatrixBuffer.mInverseClip;
		//mMatrixBuffer.mView = mat4f::lookToLH(mCamera.GetForward(), mCamera.mPosition, vec3f(0.0f, 1.0f, 0.0f)).transpose();
	}

	void VUpdate(double milliseconds) override
	{
		InitializeCamera();

		mPixelSize = { 1.0f / mRenderer->GetWindowWidth(), 1.0f / mRenderer->GetWindowWidth() };
		
		int offset = OFFSET_COUNT / 2;
		for (int i = 0; i < OFFSET_COUNT; i++) {
			mBlurH.uvOffsets[i] = { mPixelSize.x * (i - offset) , 0.0f };
			mBlurV.uvOffsets[i] = { 0.0f, mPixelSize.y * (i - offset) };
		}

		ScreenPoint mousePosition = Input::SharedInstance().mousePosition;
		if (Input::SharedInstance().GetMouseButton(MOUSEBUTTON_LEFT)) {
			mCamera.RotatePitch(-(mousePosition.y - mMouseY) * RADIAN * CAMERA_ROTATION_SPEED);
			mCamera.RotateYaw(-(mousePosition.x - mMouseX) * RADIAN * CAMERA_ROTATION_SPEED);
		}

		mMouseX = mousePosition.x;
		mMouseY = mousePosition.y;

		vec3f position = mCamera.GetPosition();
		if (Input::SharedInstance().GetKey(KEYCODE_W)) {
			position += mCamera.GetForward() * CAMERA_SPEED;
			mCamera.SetPosition(position);
		}

		if (Input::SharedInstance().GetKey(KEYCODE_A)) {
			position += mCamera.GetRight() * -CAMERA_SPEED;
			mCamera.SetPosition(position);
		}

		if (Input::SharedInstance().GetKey(KEYCODE_D)) {
			position += mCamera.GetRight() * CAMERA_SPEED;
			mCamera.SetPosition(position);
		}

		if (Input::SharedInstance().GetKey(KEYCODE_S)) {
			position += mCamera.GetForward() * -CAMERA_SPEED;
			mCamera.SetPosition(position);
		}


		if (Input::SharedInstance().GetKey(KEYCODE_1)) {
			mBlurType = BLUR_TYPE_NONE;
		}

		if (Input::SharedInstance().GetKey(KEYCODE_2)) {
			mBlurType = BLUR_TYPE_GAUSSIAN;
		}

		if (Input::SharedInstance().GetKey(KEYCODE_3)) {
			mBlurType = BLUR_TYPE_MOTION;
		}
	}

	void VRender() override
	{
		// Set up the input assembler
		mRenderer->VSetPrimitiveType(GPU_PRIMITIVE_TYPE_TRIANGLE);

		DX3D11Renderer* dxRenderer = static_cast<DX3D11Renderer*>(mRenderer);
		ID3D11DeviceContext* deviceContext = dxRenderer->GetDeviceContext();
		deviceContext->RSSetViewports(1, &dxRenderer->GetViewport());

		switch (mBlurType)
		{
		case Rig3DSampleScene::BLUR_TYPE_NONE:
			RenderNoBlur();
			break;
		case Rig3DSampleScene::BLUR_TYPE_GAUSSIAN:
			RenderGuassianBlur();
			break;
		case Rig3DSampleScene::BLUR_TYPE_MOTION:
			RenderMotionBlur();
			break;
		default:
			break;
		}

		mRenderer->VSwapBuffers();
	}

	void RenderNoBlur()
	{
		mRenderer->VSetContextTargetWithDepth();
		mRenderer->VClearContext(reinterpret_cast<const float*>(&mClearColor), 1.0f, 0);

		mRenderer->VSetInputLayout(mVertexShader);
		mRenderer->VSetVertexShader(mVertexShader);
	//	mRenderer->VSetPixelShader(mPixelShader);
		mRenderer->VSetPixelShader(mSCPixelShader);

		DrawScene();
	}

	void RenderGuassianBlur()
	{
		mRenderer->VSetRenderContextTargetWithDepth(mRenderContext, 0);
		mRenderer->VClearContext(mRenderContext, reinterpret_cast<const float*>(&mClearColor), 1.0f, 0);

		mRenderer->VSetInputLayout(mVertexShader);
		mRenderer->VSetVertexShader(mVertexShader);
		//	mRenderer->VSetPixelShader(mPixelShader);
		mRenderer->VSetPixelShader(mSCPixelShader);

		DrawScene();

		// Horizontal Pass
		// Only clear rtv here leave depth alone
		mRenderer->VSetRenderContextTarget(mRenderContext, 1);
		mRenderer->VClearContextTarget(mRenderContext, 1, reinterpret_cast<const float*>(&mClearColor));

		mRenderer->VSetInputLayout(mQuadVertexShader);
		mRenderer->VSetVertexShader(mQuadVertexShader);
		mRenderer->VSetPixelShader(mQuadBlurPixelShader);

		mRenderer->VUpdateShaderConstantBuffer(mBlurShaderResource, &mBlurH, 1);
		mRenderer->VSetPixelShaderConstantBuffer(mBlurShaderResource, 1, 0);

		mRenderer->VSetPixelShaderResourceView(mRenderContext, 0, 0);
		mRenderer->VSetPixelShaderDepthResourceView(mRenderContext, 1);
		mRenderer->VSetPixelShaderSamplerStates(mBlurShaderResource);

		ID3D11DeviceContext* deviceContext = static_cast<DX3D11Renderer*>(mRenderer)->GetDeviceContext();
		//deviceContext->PSSetSamplers(0, 1, &mSamplerState);

		mRenderer->VBindMesh(mQuadMesh);
		mRenderer->VDrawIndexed(0, mQuadMesh->GetIndexCount());

		// Final Pass
		mRenderer->VSetContextTargetWithDepth();
		mRenderer->VClearContext(reinterpret_cast<const float*>(&mClearColor), 1.0f, 0);

		mRenderer->VUpdateShaderConstantBuffer(mBlurShaderResource, &mBlurV, 1);
		mRenderer->VSetPixelShaderConstantBuffer(mBlurShaderResource, 1, 0);
		
		mRenderer->VSetPixelShaderResourceView(mRenderContext, 1, 0);
		mRenderer->VSetPixelShaderDepthResourceView(mRenderContext, 1);

		mRenderer->VDrawIndexed(0, mQuadMesh->GetIndexCount());

		ID3D11ShaderResourceView* nullSRV[2] = { 0, 0 };
		deviceContext->PSSetShaderResources(0, 2, nullSRV);
	}

	void RenderMotionBlur()
	{
		mRenderer->VSetRenderContextTargetWithDepth(mRenderContext, 0);
		mRenderer->VClearContext(mRenderContext, reinterpret_cast<const float*>(&mClearColor), 1.0f, 0);
	
		mRenderer->VSetInputLayout(mVertexShader);
		mRenderer->VSetVertexShader(mVertexShader);
		//	mRenderer->VSetPixelShader(mPixelShader);
		mRenderer->VSetPixelShader(mSCPixelShader);

		DrawScene();

		mRenderer->VSetContextTargetWithDepth();
		mRenderer->VClearContext(reinterpret_cast<const float*>(&mClearColor), 1.0f, 0);

		mRenderer->VSetInputLayout(mQuadVertexShader);
		mRenderer->VSetVertexShader(mQuadVertexShader);
		mRenderer->VSetPixelShader(mMotionBlurPixelShader);

		mRenderer->VUpdateShaderConstantBuffer(mBlurShaderResource, &mMBMatrixBuffer, 0);
		mRenderer->VSetPixelShaderConstantBuffer(mBlurShaderResource, 0, 0);

		mRenderer->VSetPixelShaderResourceView(mRenderContext, 0, 0);
		mRenderer->VSetPixelShaderDepthResourceView(mRenderContext, 1);
		mRenderer->VSetPixelShaderSamplerStates(mBlurShaderResource);

		ID3D11DeviceContext* deviceContext = static_cast<DX3D11Renderer*>(mRenderer)->GetDeviceContext();
		//deviceContext->PSSetSamplers(0, 1, &mSamplerState);

		mRenderer->VBindMesh(mQuadMesh);
		mRenderer->VDrawIndexed(0, mQuadMesh->GetIndexCount());

		ID3D11ShaderResourceView* nullSRV[2] = { 0, 0 };
		deviceContext->PSSetShaderResources(0, 2, nullSRV);
	}

	void DrawScene()
	{
		for (int i = 0; i < NODE_COUNT; i++) {
			mMatrixBuffer.mWorld = mSceneNodes[i].mTransform.GetWorldMatrix().transpose();

			mRenderer->VUpdateShaderConstantBuffer(mSphereShaderResource, &mMatrixBuffer, 0);
			mRenderer->VUpdateShaderConstantBuffer(mSphereShaderResource, &mSceneNodes[i].mColor, 1);

			mRenderer->VSetVertexShaderConstantBuffer(mSphereShaderResource, 0, 0);
			mRenderer->VSetPixelShaderConstantBuffer(mSphereShaderResource, 1, 0);
			mRenderer->VSetPixelShaderResourceView(mSphereShaderResource, 0, 0);

			ID3D11DeviceContext* deviceContext = static_cast<DX3D11Renderer*>(mRenderer)->GetDeviceContext();
			deviceContext->PSSetSamplers(0, 1, &mSamplerState);

			mRenderer->VBindMesh(mSceneNodes[i].mMesh);
			mRenderer->VDrawIndexed(0, mSceneNodes[i].mMesh->GetIndexCount());
		}
	}

	void VOnResize() override
	{
		mRenderContext->VClearRenderTargetViews();
		mRenderContext->VClearRenderTextures();
		mRenderContext->VClearShaderResourceViews();
		
		// RTV0 : SceneRTV
		// RTV1 : BlurRTV
		mRenderer->VCreateContextResourceTargets(mRenderContext, 2);
		mRenderer->VCreateContextDepthResourceTarget(mRenderContext);
	}

	void VShutdown() override
	{
		mQuadMesh->~IMesh();
		mCubeMesh->~IMesh();
		mVertexShader->~IShader();
		mPixelShader->~IShader();
		mSCPixelShader->~IShader();
		mQuadVertexShader->~IShader();
		mQuadBlurPixelShader->~IShader();
		mMotionBlurPixelShader->~IShader();
		mSphereShaderResource->~IShaderResource();
		mBlurShaderResource->~IShaderResource();
		mRenderContext->~IRenderContext();
		mAllocator.Free();
	}
};

DECLARE_MAIN(Rig3DSampleScene);