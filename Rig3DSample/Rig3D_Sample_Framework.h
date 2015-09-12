#pragma once
#include "Rig3D\Engine.h"


namespace Rig3D
{
	class Rig3D_Sample_Framework
	{
	public:
		Engine mEngine;

		virtual void Initialize();
		virtual void Update(double milliseconds) = 0;
		virtual void DrawScene() = 0;
		virtual void HandleInput() = 0;
		void Run();

		Rig3D_Sample_Framework();
		~Rig3D_Sample_Framework();
	};
}

#define DECLARE_MAIN(a)                             \
Rig3D::Rig3D_Sample_Framework *app = 0;             \
int CALLBACK WinMain(HINSTANCE hInstance,           \
                     HINSTANCE hPrevInstance,       \
                     LPSTR lpCmdLine,               \
                     int nCmdShow)                  \
{                                                   \
    a *app = new a;                                 \
    app->Run();										\
    delete app;                                     \
    return 0;                                       \
}	