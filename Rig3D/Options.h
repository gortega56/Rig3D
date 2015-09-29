#pragma once
#include "Rig3D\rig_defines.h"

#ifdef _WINDLL
#define RIG3D __declspec(dllexport)
#else
#define RIG3D __declspec(dllimport)
#endif

namespace Rig3D
{
	struct RIG3D Options
	{
		GraphicsAPI		mGraphicsAPI;
		int				mWindowWidth;
		int				mWindowHeight;
		const char*		mWindowCaption;
		bool			mFullScreen;
	};
}