#include "Rig3D_Sample_Framework.h"

using namespace Rig3D;

Rig3D_Sample_Framework::Rig3D_Sample_Framework()
{
}


Rig3D_Sample_Framework::~Rig3D_Sample_Framework()
{
}

void Rig3D_Sample_Framework::Initialize()
{
	mEngine = Engine();
	mEngine.Initialize()
}