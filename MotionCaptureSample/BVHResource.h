#pragma once
#include <stdint.h>
#include <vector>
#include <fstream>

struct BVHJoint
{
	float					Offset[3];
	uint32_t				ChannelOffset;
	uint32_t				ChannelCount;
	BVHJoint*				Parent;
	std::string				Name;
	std::vector<BVHJoint>	Children;
	std::vector<uint16_t>	ChannelOrder;
};

struct BVHHierarchy
{
	BVHJoint	Root;
	uint32_t	ChannelCount;
};

struct BVHMotion
{
	uint32_t	FrameCount;
	uint32_t	ChannelCount;
	float*		Data;
};

class BVHResource
{
public:
	BVHResource(const char* filename);
	BVHResource();
	~BVHResource();

	int Load();

private:
	const char* mFilename;

	BVHHierarchy mHierarchy;
	BVHMotion	 mMotion;
	
	void		LoadJoint(std::fstream& file, BVHJoint* joint, BVHJoint* parent = nullptr);
	int			LoadHeirachy(std::fstream& file);
	int			LoadMotion(std::fstream& file);
	void		DeleteJoint(BVHJoint* joint);
};

