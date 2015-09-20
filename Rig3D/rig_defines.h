#pragma once
#define RIG_SUCCESS		0
#define RIG_ERROR		1

#define CSTR2WSTR(in, out)					\
{											\
wchar_t wtext[256];							\
size_t num = 0;								\
mbstowcs_s(&num, wtext, in, strlen(in) + 1);\
out = wtext;								\
}

namespace Rig3D
{
	enum GRAPHICS_API
	{
		GRAPHICS_API_DIRECTX11,
		GRAPHICS_API_OPENGL4
	};

	enum GPU_MEMORY_USAGE
	{
		GPU_MEMORY_USAGE_DEFAULT,			// GPU read write, CPU read 
		GPU_MEMORY_USAGE_STATIC,			// GPU read write . CPU no access after init
		GPU_MEMORY_USAGE_DYNAMIC,			// GPU read, CPU write
		GPU_MEMORY_USAGE_COPY				// GPU write, CPU read.
	};

	enum GPU_BUFFER_TYPE
	{
		GPU_BUFFER_TYPE_VERTEX,
		GPU_BUFFER_TYPE_INDEX,
		GPU_BUFFER_TYPE_BLOCK,
		GPU_BUFFER_TYPE_RESOURCE,
		GPU_BUFFER_TYPE_TEXTURE
	};

	enum GPU_PRIMITIVE_TYPE
	{
		GPU_PRIMITIVE_TYPE_POINT,
		GPU_PRIMITIVE_TYPE_LINE,
		GPU_PRIMITIVE_TYPE_TRIANGLE,
		GPU_PRIMITIVE_TYPE_TRIANGLE_STRIP,
		GPU_PRIMITIVE_TYPE_TRIANGLE_FAN
	};
}

