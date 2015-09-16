#include <Windows.h>
#include "Rig3D\Engine.h"
#include "Rig3D\Graphics\Interface\IScene.h"

using namespace Rig3D;

class Rig3DSampleScene : public IScene
{
public:
	Rig3DSampleScene()
	{
		mWindowCaption = "Rig3D Sample";
		mWindowWidth = 800;
		mWindowHeight = 600;
	}

	~Rig3DSampleScene()
	{

	}

	void VInitialize() override
	{

	}

	void VUpdate(double milliseconds) override
	{

	}

	void VRender() override
	{

	}

	void VHandleInput() override
	{

	}


};


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	Rig3D::Engine Rig3DEngine = Rig3D::Engine();
	Rig3DEngine.Initialize(hInstance, prevInstance, cmdLine, showCmd, 300, 300, "Test");
}