#pragma once
#include <Windows.h>
#include <stdint.h>
#include "Rig3D\Options.h"
#include "Memory/Memory/Memory.h"

#ifdef _WINDLL
#define RIG3D __declspec(dllexport)
#else
#define RIG3D __declspec(dllimport)
#endif


namespace Rig3D 
{
	class IScene;
	class IMesh;
	class IShader;

	class RIG3D IRendererDelegate
	{
	public:
		virtual void VOnResize() = 0;

		IRendererDelegate() {};
		virtual ~IRendererDelegate() {};
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

		virtual void	VSetPrimitiveType(GPUPrimitiveType type) = 0;
		virtual void	VDrawIndexed(GPUPrimitiveType type, uint32_t startIndex, uint32_t count) = 0;
		virtual void    VDrawIndexed(uint32_t startIndex, uint32_t count) = 0;

		virtual void	VSetMeshVertexBufferData(IMesh* mesh, void* vertices, const size_t& size, const size_t& stride, const GPUMemoryUsage& usage) = 0;
		virtual void	VSetMeshIndexBufferData(IMesh* mesh, uint16_t* indices, const uint32_t& count, const GPUMemoryUsage& usage) = 0;
		virtual void    VBindMesh(IMesh* mesh) = 0;

		virtual void	VLoadVertexShader(IShader* vertexShader, const char* filename, LinearAllocator& allocator) = 0;
		virtual void	VLoadPixelShader(IShader* pixelShader, const char* filename) = 0;

		virtual void	VSetVertexShader(IShader* shader) = 0;
		virtual void	VSetPixelShader(IShader* shader) = 0;

		virtual void    VSwapBuffers() = 0;

		void SetWindowCaption(const char* caption);
		
		inline float		GetAspectRatio() const { return (float)mWindowWidth / mWindowHeight; };
		inline const int&	GetWindowWidth() const { return mWindowWidth; };
		inline const int&	GetWindowHeight() const { return mWindowHeight; };
		inline GraphicsAPI GetGraphicsAPI() const { return mGraphicsAPI; };
	
		inline void SetDelegate(IRendererDelegate* renderDelegate) { mDelegate = renderDelegate; };

	protected:
		HINSTANCE			mHINSTANCE;
		HWND				mHWND;
		int					mWindowWidth;
		int					mWindowHeight;
		GraphicsAPI			mGraphicsAPI;
		IRendererDelegate*	mDelegate;
		const char*			mWindowCaption;		
		bool				mFullScreen;
	};
}

