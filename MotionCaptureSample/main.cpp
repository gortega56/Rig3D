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

using namespace Rig3D;

class MotionCaptureSample : public IScene, public virtual IRendererDelegate
{
public:
	LinearAllocator mMeshAllocator;


	void VInitialize() override
	{

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
};