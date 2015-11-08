#include <Windows.h>
#include "Rig3D\Engine.h"
#include "Rig3D\Graphics\Interface\IScene.h"
#include "Rig3D\Graphics\DirectX11\DX3D11Renderer.h"
#include "Rig3D\Graphics\Interface\IMesh.h"
#include "Rig3D\Common\Transform.h"
#include "Memory\Memory\Memory.h"
#include "Rig3D\Graphics\MeshLibrary.h"
#include <d3d11.h>
#include <fstream>
#include "Rig3D\Graphics\Interface\IShader.h"
#include "BVHResource.h"

using namespace Rig3D;

class MotionCaptureSample : public IScene, public virtual IRendererDelegate
{
public:
	LinearAllocator mMeshAllocator;

	MotionCaptureSample();
	~MotionCaptureSample();

	void VInitialize() override;
	void VUpdate(double milliseconds);
	void VRender() override;
	void VShutdown() override;
	void VOnResize() override;

	void InitializeGeometry();
	void InitializeBVHResources();
};

MotionCaptureSample::MotionCaptureSample() : mMeshAllocator(1024)
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
	InitializeGeometry();
	InitializeBVHResources();
}

void MotionCaptureSample::VUpdate(double milliseconds)
{

}

void MotionCaptureSample::VRender()
{

}

void MotionCaptureSample::VShutdown()
{
	mMeshAllocator.Free();
}

void MotionCaptureSample::VOnResize()
{

}

void MotionCaptureSample::InitializeGeometry()
{

}

void MotionCaptureSample::InitializeBVHResources()
{
	BVHResource bvhResource = BVHResource("BVH\\Sneak.bvh");
	bvhResource.Load();
}

DECLARE_MAIN(MotionCaptureSample);