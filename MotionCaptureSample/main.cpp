#include <Windows.h>
#include "Rig3D\Engine.h"
#include "Rig3D\Graphics\Interface\IScene.h"
#include "Rig3D\Graphics\DirectX11\DX3D11Renderer.h"
#include "Rig3D\Graphics\Interface\IMesh.h"
#include "Rig3D\Common\Transform.h"
#include "Memory\Memory\Memory.h"
#include "Rig3D\Graphics\MeshLibrary.h"
#include <d3d11.h>
#include "Rig3D\Graphics\Interface\IShader.h"
#include "BVHResource.h"
#include "Rig3D\Graphics\DirectX11\DX11Shader.h"


#define PI						3.1415926535f
#define CAMERA_SPEED			0.1f
#define CAMERA_ROTATION_SPEED	0.1f
#define RADIAN					3.1415926535f / 180.0f

using namespace Rig3D;

struct Vertex3
{
	vec3f Position;
	vec3f Normal;
	vec2f UV;
};

struct ModelViewProjection
{
	mat4f View;
	mat4f Projection;
};

class MotionCaptureSample : public IScene, public virtual IRendererDelegate
{
public:
	ModelViewProjection	mViewProjection;
	mat4f*				mJointWorldMatrices;

	Transform			mCamera;
	BVHResource			mBVHResource;

	LinearAllocator		mAllocator;

	Transform*			mTransforms;
	IMesh*				mCubeMesh;
	IMesh*				mPyramidMesh;

	IRenderer*			mRenderer;
	IShader*			mVertexShader;
	IShader*			mPixelShader;

	uint32_t			mTransformCount;

	float				mMouseX;
	float				mMouseY;

	MotionCaptureSample();
	~MotionCaptureSample();

	void VInitialize() override;
	void VUpdate(double milliseconds) override;
	void VRender() override;
	void VShutdown() override;
	void VOnResize() override;

	void InitializeGeometry();
	void InitializeBVHResources();
	void InitializeTransforms(Transform* transforms, const BVHJoint* joint);

	void InitializeShaders();

	void UpdateCamera();
	void UpdateTransforms(Transform* transforms, const BVHJoint* joint, const BVHMotion* motion, const uint32_t& transformCount, const uint32_t& frameIndex);
	void UpdateJointWorldMatrices(mat4f* jointWorldMatrices, Transform* transforms, const uint32_t& count);
	void HandleInput(Input& input);
};

MotionCaptureSample::MotionCaptureSample() : 
	mJointWorldMatrices(nullptr),
	mAllocator(8000),
	mTransforms(nullptr), 
	mCubeMesh(nullptr), 
	mPyramidMesh(nullptr),
	mRenderer(nullptr),
	mVertexShader(nullptr),
	mPixelShader(nullptr),
	mTransformCount(0),
	mMouseX(0.0f),
	mMouseY(0.0f)
{
	mOptions.mWindowCaption = "Motion Capture Sample";
	mOptions.mWindowWidth = 1200;
	mOptions.mWindowHeight = 900;
	mOptions.mGraphicsAPI = GRAPHICS_API_DIRECTX11;
	mOptions.mFullScreen = false;
}

MotionCaptureSample::~MotionCaptureSample()
{

}

void MotionCaptureSample::VInitialize()
{
	mRenderer = &DX3D11Renderer::SharedInstance();

	mCamera.SetPosition(vec3f(0.0f, 0.0f, -100.0f));
	
	InitializeGeometry();
	InitializeBVHResources();
	InitializeShaders();
}

void MotionCaptureSample::VUpdate(double milliseconds)
{
	static uint32_t frame = 0;

	HandleInput(Input::SharedInstance());
	UpdateCamera();
	UpdateTransforms(mTransforms, &mBVHResource.mHierarchy.Root, &mBVHResource.mMotion, mTransformCount, frame * mBVHResource.mMotion.ChannelCount);
	UpdateJointWorldMatrices(mJointWorldMatrices, mTransforms, mTransformCount);

	frame++;
	if (frame == mBVHResource.mMotion.FrameCount)
	{
		frame = 0;
	}
}

void MotionCaptureSample::VRender()
{
	DX3D11Renderer* renderer = reinterpret_cast<DX3D11Renderer*>(mRenderer);
	ID3D11DeviceContext* deviceContext = renderer->GetDeviceContext();

	float color[4] = { 0.0f, 0.7294117647f, 1.0f, 1.0f };
	deviceContext->OMSetRenderTargets(1, renderer->GetRenderTargetView(), renderer->GetDepthStencilView());
	deviceContext->ClearRenderTargetView(*renderer->GetRenderTargetView(), color);
	deviceContext->ClearDepthStencilView(renderer->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	deviceContext->RSSetViewports(1, &renderer->GetViewport());

	mRenderer->VSetPrimitiveType(GPU_PRIMITIVE_TYPE_TRIANGLE);
	mRenderer->VSetInputLayout(mVertexShader);
	mRenderer->VSetInstanceBuffers(mVertexShader);
	mRenderer->VSetVertexShader(mVertexShader);
	mRenderer->VSetPixelShader(mPixelShader);

	mRenderer->VUpdateShaderConstantBuffer(mVertexShader, &mViewProjection, 0);
	mRenderer->VSetVertexShaderResources(mVertexShader);
	mRenderer->VBindMesh(mCubeMesh);
	deviceContext->DrawIndexedInstanced(mCubeMesh->GetIndexCount(), mTransformCount, 0, 0, 0);

	mRenderer->VSwapBuffers();
}

void MotionCaptureSample::VShutdown()
{
	mCubeMesh->~IMesh();
	mPyramidMesh->~IMesh();
	mVertexShader->~IShader();
	mPixelShader->~IShader();
	mAllocator.Free();
}

void MotionCaptureSample::VOnResize()
{

}

void MotionCaptureSample::InitializeGeometry()
{
	MeshLibrary<LinearAllocator> meshLibrary;
	meshLibrary.SetAllocator(&mAllocator);

	OBJBasicResource<Vertex3> cubeResource("Models\\cube.obj");
	OBJBasicResource<Vertex3> pyramidResource("Models\\pyramid.obj");

	meshLibrary.LoadMesh(&mCubeMesh, mRenderer, cubeResource);
	meshLibrary.LoadMesh(&mPyramidMesh, mRenderer, pyramidResource);
}

void MotionCaptureSample::InitializeBVHResources()
{
	mBVHResource.SetFilename("BVH\\Stand.bvh");
	mBVHResource.Load();

	mTransformCount = mBVHResource.mHierarchy.JointCount;

	mTransforms = reinterpret_cast<Transform*>(mAllocator.Allocate(sizeof(Transform) * mTransformCount, alignof(Transform), 0));
	memset(mTransforms, 0, sizeof(Transform) * mTransformCount);

	mJointWorldMatrices = reinterpret_cast<mat4f*>(mAllocator.Allocate(sizeof(mat4f) * mTransformCount, alignof(mat4f), 0));
	memset(mJointWorldMatrices, 0, sizeof(mat4f) * mTransformCount);

	BVHJoint* currentJoint = &mBVHResource.mHierarchy.Root;
	InitializeTransforms(mTransforms, currentJoint);
}

void MotionCaptureSample::InitializeTransforms(Transform* transforms, const BVHJoint* joint)
{	
	static uint32_t index = 0;

	Transform* parent = &transforms[index++];
	parent->SetScale(vec3f(1.0f));
	
	for (uint32_t i = 0; i < joint->Children.size(); i++)
	{
		transforms[index].SetParent(parent);
		InitializeTransforms(transforms, &joint->Children[i]);
	}
}

void MotionCaptureSample::InitializeShaders()
{
	InputElement inputElements[] =
	{
		{ "POSITION",	0, 0, 0,  0, FLOAT3, INPUT_CLASS_PER_VERTEX },
		{ "NORMAL",		0, 0, 12, 0, FLOAT3, INPUT_CLASS_PER_VERTEX },
		{ "TEXCOORD",	0, 0, 24, 0, FLOAT2, INPUT_CLASS_PER_VERTEX },
		{ "WORLD",		0, 1, 0, 1, FLOAT4, INPUT_CLASS_PER_INSTANCE },
		{ "WORLD",		1, 1, 16, 1, FLOAT4, INPUT_CLASS_PER_INSTANCE },
		{ "WORLD",		2, 1, 32, 1, FLOAT4, INPUT_CLASS_PER_INSTANCE },
		{ "WORLD",		3, 1, 48, 1, FLOAT4, INPUT_CLASS_PER_INSTANCE }
	};

	// Load Vertex Shader --------------------------------------

	mRenderer->VCreateShader(&mVertexShader, &mAllocator);
	mRenderer->VLoadVertexShader(mVertexShader, "MCVertexShader.cso", inputElements, 7);

	// Load Pixel Shader ---------------------------------------

	mRenderer->VCreateShader(&mPixelShader, &mAllocator);
	mRenderer->VLoadPixelShader(mPixelShader, "MCPixelShader.cso");

	// Constant buffers ----------------------------------------

	void*	constantBufferData[]	= { &mViewProjection };
	size_t	constantBufferSizes[]	= { sizeof(ModelViewProjection) };
	mRenderer->VCreateShaderConstantBuffers(mVertexShader, constantBufferData, constantBufferSizes, 1);

	// Instance buffers -----------------------------------------

	void*	instanceBufferData[]	= { mJointWorldMatrices };
	size_t	instanceBufferSizes[]	= { sizeof(mat4f) * mTransformCount };
	size_t	instanceBufferStrides[] = { sizeof(mat4f) };
	size_t	instanceBufferOffsets[] = { 0 };
	mRenderer->VCreateDynamicShaderInstanceBuffers(mVertexShader, instanceBufferData, instanceBufferSizes, instanceBufferStrides, instanceBufferOffsets, 1);
}

void MotionCaptureSample::UpdateCamera()
{
	mViewProjection.View = mat4f::lookAtLH(mCamera.GetPosition() + mCamera.GetForward(), mCamera.GetPosition(), mCamera.GetUp()).transpose();
	mViewProjection.Projection = mat4f::normalizedPerspectiveLH(PI * 0.25f, mRenderer->GetAspectRatio(), 0.1f, 500.0f).transpose();
}

void MotionCaptureSample::UpdateTransforms(Transform* transforms, const BVHJoint* joint, const BVHMotion* motion, const uint32_t& transformCount, const uint32_t& frameIndex)
{
	static uint32_t index = 0;

	if (index == transformCount)
	{
		index = 0;
	}

	uint32_t channelOffset = frameIndex + joint->ChannelOffset;
	vec3f jointPosition = { joint->Offset[0], joint->Offset[1], joint->Offset[2] };
	vec3f jointRotation = { 0.0f, 0.0f, 0.0f };

	for (uint32_t i = 0; i < joint->ChannelCount; i++)
	{
		uint16_t channel = joint->ChannelOrder[i];
		float value = motion->Data[channelOffset + i];

		if (channel & DOF_POSITION_X)
		{
			jointPosition.x += value;
		}
		else if (channel & DOF_POSITION_Y)
		{
			jointPosition.y += value;
		}
		else if (channel & DOF_POSITION_Z)
		{
			jointPosition.z += value;
		}
		else if (channel & DOF_ROTATION_X)
		{
			jointRotation.x += value * RADIAN;
		}
		else if (channel & DOF_ROTATION_Y)
		{
			jointRotation.y += value * RADIAN;
		}
		else if (channel & DOF_ROTATION_Z)
		{
			jointRotation.z += value * RADIAN;
		}
	}

	Transform* transform = &transforms[index++];
	transform->SetPosition(jointPosition);
	transform->SetRotation(jointRotation);

	for (uint32_t i = 0; i < joint->Children.size(); i++)
	{
		UpdateTransforms(transforms, &joint->Children[i], motion, transformCount, frameIndex);
	}
}

void MotionCaptureSample::UpdateJointWorldMatrices(mat4f* jointWorldMatrices, Transform* transforms, const uint32_t& count)
{
	for (uint32_t i = 0; i < count; i++)
	{
		mJointWorldMatrices[i] = transforms[i].GetWorldMatrix().transpose();
	}

	mRenderer->VUpdateShaderInstanceBuffer(mVertexShader, mJointWorldMatrices, sizeof(mat4f) * count, 0);
}

void MotionCaptureSample::HandleInput(Input& input)
{
	ScreenPoint mousePosition = Input::SharedInstance().mousePosition;
	if (input.GetMouseButton(MOUSEBUTTON_LEFT)) {
		mCamera.RotatePitch(-(mousePosition.y - mMouseY) * RADIAN * CAMERA_ROTATION_SPEED);
		mCamera.RotateYaw(-(mousePosition.x - mMouseX) * RADIAN * CAMERA_ROTATION_SPEED);
	}

	mMouseX = mousePosition.x;
	mMouseY = mousePosition.y;

	vec3f position = mCamera.GetPosition();
	if (input.GetKey(KEYCODE_W)) {
		position += mCamera.GetForward() * CAMERA_SPEED;
		mCamera.SetPosition(position);
	}

	if (input.GetKey(KEYCODE_A)) {
		position += mCamera.GetRight() * -CAMERA_SPEED;
		mCamera.SetPosition(position);
	}

	if (input.GetKey(KEYCODE_D)) {
		position += mCamera.GetRight() * CAMERA_SPEED;
		mCamera.SetPosition(position);
	}

	if (input.GetKey(KEYCODE_S)) {
		position += mCamera.GetForward() * -CAMERA_SPEED;
		mCamera.SetPosition(position);
	}
}

DECLARE_MAIN(MotionCaptureSample);