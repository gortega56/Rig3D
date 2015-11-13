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

using namespace Rig3D;

uint8_t				gMemory[1024];

class PhysicallyBasedLightingSample : public IScene, public virtual IRendererDelegate
{
public:
	Transform			mCamera;
	LinearAllocator		mAllocator;

	mat4f*				mSphereWorldMatrices;
	Transform*			mTransforms;
	IMesh*				mSphereMesh;

	IRenderer*			mRenderer;
	IShader*			mPBLModelVertexShader;
	IShader*			mPBLModelPixelShader;

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

	void UpdateCamera();
	void HandleInput(Input& input);
};

PhysicallyBasedLightingSample::PhysicallyBasedLightingSample() :
	mAllocator(gMemory, gMemory + 1024),
	mSphereWorldMatrices(nullptr),
	mTransforms(nullptr),
	mSphereMesh(nullptr),
	mRenderer(nullptr),
	mPBLModelVertexShader(nullptr),
	mPBLModelPixelShader(nullptr),
	mMouseX(0.0f),
	mMouseY(0.0f)
{

}

PhysicallyBasedLightingSample::~PhysicallyBasedLightingSample()
{
	
}

void PhysicallyBasedLightingSample::VInitialize()
{
	
}

void PhysicallyBasedLightingSample::VUpdate(double milliseconds)
{
	
}

void PhysicallyBasedLightingSample::VRender()
{
	
}

void PhysicallyBasedLightingSample::VShutdown()
{
	
}

void PhysicallyBasedLightingSample::VOnResize()
{
	
}

void PhysicallyBasedLightingSample::InitializeGeometry()
{
	
}

void PhysicallyBasedLightingSample::InitializeShaders()
{
	
}

void PhysicallyBasedLightingSample::UpdateCamera()
{
	
}

void PhysicallyBasedLightingSample::HandleInput(Input& input)
{
	
}