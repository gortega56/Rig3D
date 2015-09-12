#include <Windows.h>
#include "Rig3D\Engine.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	Rig3D::Engine Rig3DEngine = Rig3D::Engine();
	Rig3DEngine.Initialize(hInstance, prevInstance, cmdLine, showCmd);
}