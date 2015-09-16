#pragma once
#include <Windows.h>

#ifdef _WINDLL
#define RIG3D __declspec(dllexport)
#else
#define RIG3D __declspec(dllimport)
#endif

namespace Rig3D 
{
	class RIG3D IRenderer
	{
	public:
		virtual int VInitialize(HINSTANCE hInstance, HWND hwnd, int windowWidth, int windowHeight, const char* windowCaption) = 0;
		virtual void VOnResize() = 0;
		virtual void VUpdateScene(const double& milliseconds) = 0;
		virtual void VRenderScene() = 0;

		inline void SetWindowCaption(const char* caption) { mWindowCaption = caption; };
		inline float GetAspectRatio() const { return (float)mWindowWidth / mWindowHeight; };

		IRenderer(int windowWidth, int windowHeight, const char* windowCaption) : mWindowWidth(windowWidth), mWindowHeight(windowHeight), mWindowCaption(windowCaption) {};
		IRenderer();
		~IRenderer();
		
	protected:
		int			mWindowWidth;
		int			mWindowHeight;
		const char* mWindowCaption;
	};
}

