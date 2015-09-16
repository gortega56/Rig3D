#pragma once

#ifdef _WINDLL
#define RIG3D __declspec(dllexport)
#else
#define RIG3D __declspec(dllimport)
#endif

namespace Rig3D
{
	class RIG3D IScene
	{
	public:
		int			mWindowWidth;
		int			mWindowHeight;
		const char* mWindowCaption;

		IScene();
		~IScene();

		virtual void VInitialize() = 0;
		virtual void VUpdate(double milliseconds) = 0;
		virtual void VRender() = 0;
		virtual void VHandleInput() = 0;
	};
}

// Leaving this here for now. Will integrate later.
//#define DECLARE_MAIN(a)                             \
//Rig3D::Rig3D_Sample_Framework *app = 0;             \
//int CALLBACK WinMain(HINSTANCE hInstance,           \
//                     HINSTANCE hPrevInstance,       \
//                     LPSTR lpCmdLine,               \
//                     int nCmdShow)                  \
//{                                                   \
//    a *app = new a;                                 \
//    app->Run();										\
//    delete app;                                     \
//    return 0;                                       \
//}	