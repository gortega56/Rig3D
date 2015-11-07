#pragma once
#include <stdint.h>
#include <fstream>

struct BVHJoint
{
	float		Offset[3];
	uint32_t	ChannelOffset;
	uint32_t	ChannelCount;
	uint8_t		DOF;
	std::string	Name;
	BVHJoint*	Parent;
};

struct BVHHierarchy
{
	BVHJoint*	Root;
	uint32_t	ChannelCount;
};

struct BVHMotion
{
	uint32_t	FrameCount;
	uint32_t	ChannelCount;
	uint32_t*	ChannelOffsets;
	float*		Data;
};

class BVHResource
{
public:
	const char* mFilename;

	BVHResource(const char* filename);
	BVHResource();
	~BVHResource();

	int Load();

private:
	BVHHierarchy mHierarchy;
	BVHMotion	 mMotion;
	
	BVHJoint*	LoadJoint(std::fstream& file, BVHJoint* parent = nullptr);
	int			LoadHeirachy(std::fstream& file);
	int			LoadMotion(std::fstream& file);
};

