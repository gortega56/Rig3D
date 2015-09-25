#pragma once
#include <Windows.h>
#include <stdint.h>
#include "Rig3D\Options.h"

#ifdef _WINDLL
#define RIG3D __declspec(dllexport)
#else
#define RIG3D __declspec(dllimport)
#endif

namespace Rig3D 
{
	class IScene;

	class RIG3D IRendererDelegate
	{
	public:
		virtual void VOnResize() = 0;

		IRendererDelegate() {};
		~IRendererDelegate() {};
	};

	class RIG3D IRenderer
	{
	public:
		IRenderer();
		virtual ~IRenderer();
		
		virtual int		VInitialize(HINSTANCE hInstance, HWND hwnd, Options options) = 0;
		virtual void	VOnResize() = 0;
		virtual void	VUpdateScene(const double& milliseconds) = 0;
		virtual void	VRenderScene() = 0;
		virtual void	VShutdown() = 0;

		virtual void	VSetPrimitiveType(GPU_PRIMITIVE_TYPE type) = 0;
		virtual void	VDrawIndexed(GPU_PRIMITIVE_TYPE type, uint32_t startIndex, uint32_t count) = 0;
		virtual void    VDrawIndexed(uint32_t startIndex, uint32_t count) = 0;

		void SetWindowCaption(const char* caption);
		
		inline float		GetAspectRatio() const { return (float)mWindowWidth / mWindowHeight; };
		inline const int&	GetWindowWidth() const { return mWindowWidth; };
		inline const int&	GetWindowHeight() const { return mWindowHeight; };
		inline GRAPHICS_API GetGraphicsAPI() const { return mGraphicsAPI; };
	
		inline void SetDelegate(IRendererDelegate* renderDelegate) { mDelegate = renderDelegate; };

	protected:
		HINSTANCE			mHINSTANCE;
		HWND				mHWND;
		int					mWindowWidth;
		int					mWindowHeight;
		GRAPHICS_API		mGraphicsAPI;
		IRendererDelegate*	mDelegate;
		const char*			mWindowCaption;		
		bool				mFullScreen;
	};
}

