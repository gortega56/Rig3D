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
	class IRenderContext;

	enum InputClass
	{
		INPUT_CLASS_PER_VERTEX,
		INPUT_CLASS_PER_INSTANCE
	};

	enum InputFormat
	{
		R_FLOAT32,
		RG_FLOAT32,
		RGB_FLOAT32,
		RGBA_FLOAT32,
		R_SNORM8,
		RG_SNORM8,
		RGBA_SNORM8,
		R_TYPELESS8,
		RG_TYPELESS8,
		RGBA_TYPELESS8
	};

	struct InputElement
	{
		const char* Name;
		uint32_t	Index;
		uint32_t	InputSlot;
		uint32_t	ByteOffset;
		uint32_t	InstanceStepRate;
		InputFormat	Format;
		InputClass	InputSlotClass;
	};

	enum TextureFormat
	{

	};

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

#pragma region Buffer

		virtual void	VCreateVertexBuffer(void* buffer, void* vertices, const size_t& size) = 0;
		virtual void	VCreateStaticVertexBuffer(void* buffer, void* vertices, const size_t& size) = 0;
		virtual void	VCreateDynamicVertexBuffer(void* buffer, void* vertices, const size_t& size) = 0;

		virtual void	VCreateIndexBuffer(void* buffer, uint16_t* indices, const uint32_t& count) = 0;
		virtual void	VCreateStaticIndexBuffer(void* buffer, uint16_t* indices, const uint32_t& count) = 0;
		virtual void	VCreateDynamicIndexBuffer(void* buffer, uint16_t* indices, const uint32_t& count) = 0;

		virtual void	VCreateInstanceBuffer(void* buffer, void* data, const size_t& size) = 0;
		virtual void	VCreateStaticInstanceBuffer(void* buffer, void* data, const size_t& size) = 0;
		virtual void	VCreateDynamicInstanceBuffer(void* buffer, void* data, const size_t& size) = 0;

		virtual void	VCreateConstantBuffer(void* buffer, void* data, const size_t& size) = 0;
		virtual void	VCreateStaticConstantBuffer(void* buffer, void* data, const size_t& size) = 0;
		virtual void	VCreateDynamicConstantBuffer(void* buffer, void* data, const size_t& size) = 0;

		virtual void	VUpdateBuffer(void* buffer, void* data) = 0;						// Copies data of predetermined size
		virtual void	VUpdateBuffer(void* buffer, void* data, const size_t& size) = 0;	// Map - Copy - Unmap

#pragma endregion

#pragma region Texture

		virtual void VCreateNativeFormat(void* nativeFormat, InputFormat format) = 0;
		virtual void VCreateTexture2D(void* texture, void* data, uint32_t mipLevels, InputFormat textureFormat) = 0;

		// 1 Channel (8 Bit) Texture.
		virtual void VCreateDepthTexture2D(void* texture2D) = 0;
		virtual void VCreateDepthResourceTexture2D(void * texture2D) = 0;

		virtual void VCreateDepthStencilTexture2D(void* texture2D) = 0;
		virtual void VCreateDepthStencilResourceTexture2D(void * texture2D) = 0;

		// 4 Channel (32 Bit) Texture.
		virtual void VCreateRenderTexture2D(void* texture2D) = 0;
		virtual void VCreateRenderResourceTexture2D(void * texture2D) = 0;

#pragma endregion 

#pragma region Mesh

		virtual void	VSetMeshVertexBuffer(IMesh* mesh, void* vertices, const size_t& size, const size_t& stride) = 0;
		virtual void	VSetStaticMeshVertexBuffer(IMesh* mesh, void* vertices, const size_t& size, const size_t& stride) = 0;
		virtual void	VSetDynamicMeshVertexBuffer(IMesh* mesh, void* vertices, const size_t& size, const size_t& stride) = 0;

		virtual void	VSetMeshIndexBuffer(IMesh* mesh, uint16_t* indices, const uint32_t& count) = 0;
		virtual void	VSetStaticMeshIndexBuffer(IMesh* mesh, uint16_t* indices, const uint32_t& count) = 0;
		virtual void	VSetDynamicMeshIndexBuffer(IMesh* mesh, uint16_t* indices, const uint32_t& count) = 0;

		virtual void	VUpdateMeshVertexBuffer(IMesh* mesh, void* data, const size_t& size) = 0;
		virtual void	VUpdateMeshIndexBuffer(IMesh* mesh, void* data, const uint32_t& count) = 0;

		virtual void    VBindMesh(IMesh* mesh) = 0;

#pragma endregion 

#pragma region Shaders

		virtual void	VCreateShader(IShader** shader, LinearAllocator* allocator) = 0;
		virtual void	VLoadVertexShader(IShader* vertexShader, const char* filename, InputElement* inputElements, const uint32_t& count) = 0;
		virtual void	VLoadVertexShader(IShader* vertexShader, const char* filename, LinearAllocator* allocator) = 0;
		virtual void	VLoadVertexShader(IShader* vertexShader, const char* filename) = 0;

		virtual void	VLoadPixelShader(IShader* vertexShader, const char* filename, LinearAllocator* allocator) = 0;
		virtual void	VLoadPixelShader(IShader* pixelShader, const char* filename) = 0;

		virtual void	VSetInputLayout(IShader* vertexShader) = 0;
		virtual void	VSetInstanceBuffers(IShader* vertexShader) = 0;
		virtual void	VSetVertexShaderInputLayout(IShader* vertexShader) = 0;
		virtual void	VSetVertexShader(IShader* shader) = 0;
		
		virtual void	VSetPixelShader(IShader* shader) = 0;

		virtual void	VSetShaderResources(IShader* shader) = 0;

		// Resource View will be bound to GPU registers in the order they are arranged starting at input slot 0
		virtual void	VLoadShaderTextures2D(IShader* shader, const char** filenames, const uint32_t count) = 0;
		virtual void	VLoadShaderTextureCubes(IShader* shader, const char** filenames, const uint32_t count) = 0;

		virtual void	VCreateShaderResourceViews(IShader* shader, void** resourceViews, const uint32_t& count) = 0;

		// Constant buffers will be bound to GPU registers in the order they are arranged starting at input slot 0.
		virtual void	VCreateShaderConstantBuffers(IShader* shader, void** data, size_t* sizes, const uint32_t& count) = 0;

		// Instance buffers will be bound to GPU registers in the order they are arranged starting at input slot 1.
		virtual void	VCreateShaderInstanceBuffers(IShader* shader, void** data, size_t* sizes, size_t* strides, size_t* offsets, const uint32_t& count) = 0;
		virtual void	VCreateStaticShaderInstanceBuffers(IShader* shader, void** data, size_t* sizes, size_t* strides, size_t* offsets, const uint32_t& count) = 0;
		virtual void	VCreateDynamicShaderInstanceBuffers(IShader* shader, void** data, size_t* sizes, size_t* strides, size_t* offsets, const uint32_t& count) = 0;

		// Index is result of the order the instance buffers were arranged when setting the shader.
		virtual void	VUpdateShaderConstantBuffer(IShader* shader, void* data, const uint32_t& index) = 0;
		virtual void	VUpdateShaderInstanceBuffer(IShader* shader, void* data, const size_t& size, const uint32_t& index) = 0;

#pragma endregion

#pragma region Render Context

		virtual void VCreateContextDepthStencilTarget(IRenderContext* renderContext) = 0;
		virtual void VCreateContextTargets(IRenderContext* renderContext, const uint32_t& count) = 0;

		virtual void VCreateContextDepthResourceTarget(IRenderContext* renderContext) = 0;
		virtual void VCreateContextResourceTargets(IRenderContext* renderContext, const uint32_t& count) = 0;

		virtual void VSetRenderContext(IRenderContext* renderContext) = 0;

#pragma endregion 
		virtual void    VSwapBuffers() = 0;

		void SetWindowCaption(const char* caption);
		
		inline float		GetAspectRatio() const { return static_cast<float>(mWindowWidth) / mWindowHeight; };
		inline const int&	GetWindowWidth() const { return mWindowWidth; };
		inline const int&	GetWindowHeight() const { return mWindowHeight; };
		inline GraphicsAPI	GetGraphicsAPI() const { return mGraphicsAPI; };
	
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

