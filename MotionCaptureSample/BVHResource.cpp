// References: http://www.gamedev.net/page/resources/_/technical/game-programming/bvh-file-loading-and-displaying-r3295

#include "BVHResource.h"
#include <string>
#include <fstream>
#include "StringTrim.h"
#include <sstream>

static const std::string BVHKeyHierarchy	("HIERARCHY");
static const std::string BVHKeyRoot			("ROOT");
static const std::string BVHKeyOffset		("OFFSET");
static const std::string BVHKeyChannels		("CHANNELS");
static const std::string BVHKeyJoint		("JOINT");
static const std::string BVHKeyMotion		("MOTION");
static const std::string BVHKeyPositionX	("Xposition");
static const std::string BVHKeyPositionY	("Yposition");
static const std::string BVHKeyPositionZ	("Zposition");
static const std::string BVHKeyRotationX	("Xrotation");
static const std::string BVHKeyRotationY	("Yrotation");
static const std::string BVHKeyRotationZ	("Zrotation");
static const std::string BVHKeyEndSite		("End");
static const std::string BVHKeyFrameCount	("Frames:");
static const std::string BVHKeyFrame        ("Frame");

static const char BVHBeginElement	= '{';
static const char BVHEndElement		= '}';

static const uint8_t DOF_POSITION_X = 1;
static const uint8_t DOF_POSITION_Y = 1 << 1;
static const uint8_t DOF_POSITION_Z = 1 << 2;
static const uint8_t DOF_ROTATION_X = 1 << 3;
static const uint8_t DOF_ROTATION_Y = 1 << 4;
static const uint8_t DOF_ROTATION_Z = 1 << 5;

BVHResource::BVHResource(const char* filename) : mFilename(filename)
{

}

BVHResource::BVHResource() : BVHResource(nullptr)
{

}

BVHResource::~BVHResource()
{
}

int BVHResource::Load()
{
	std::fstream file(mFilename);

	if (!file.is_open())
	{
		return -1;
	}

	std::string line;

	file >> line;

	if (Trim(line) == BVHKeyHierarchy)
	{
		LoadHeirachy(file);
	}

	file.close();

	return 1;
}

BVHJoint* BVHResource::LoadJoint(std::fstream& file, BVHJoint* parent)
{
	BVHJoint* joint = new BVHJoint;
	joint->Parent = parent;

	// Joint name.
	file >> joint->Name;

	std::string line;

	static int	channelStart	= 0;
	uint32_t channelOrderIndex	= 0;

	while (file.good())
	{
		file >> line;

		line = Trim(line);
	
		char c = line.at(0);
		if (c == 'X' || c == 'Y' || c == 'Z')
		{
			if (line == BVHKeyPositionX)
			{
				
			}
			else if (line == BVHKeyPositionY)
			{

			}
			else if (line == BVHKeyPositionZ)
			{

			}
			else if (line == BVHKeyRotationX)
			{

			}
			else if (line == BVHKeyRotationY)
			{

			}
			else if (line == BVHKeyRotationZ)
			{

			}
		}

		if (line == BVHKeyOffset)
		{
			file >> joint->Offset[0] >> joint->Offset[1] >> joint->Offset[2];
		}
		else if (line == BVHKeyChannels)
		{
			file >> joint->ChannelCount;
		
			mMotion.ChannelCount += joint->ChannelCount;

			joint->ChannelOffset = channelStart;
			channelStart += joint->ChannelCount;

			// set up channel order storage for next read	
		}
		else if (line == BVHKeyJoint)
		{
			LoadJoint(file, joint);

			// Maybe push back into list of children?
			// Would like to not have to use child list here.
		}
		else if (line == BVHKeyEndSite)
		{
			file >> line >> line;

			BVHJoint* leafJoint = new BVHJoint;
			leafJoint->Parent = joint;
			leafJoint->ChannelCount = 0;
			leafJoint->Name = "End Site";
			
			file >> line;
			if (line == BVHKeyOffset)
			{
				file >> leafJoint->Offset[0] >> leafJoint->Offset[1] >> leafJoint->Offset[2];
			}

			file >> line;
		}
		else if (line == "}")
		{
			break;
		}
	}

	return joint;
}

int	BVHResource::LoadHeirachy(std::fstream& file)
{
	std::string line;

	while (file.good())
	{
		file >> line;

		if (Trim(line) == BVHKeyRoot)
		{
			mHierarchy.Root = LoadJoint(file);
		}
		else if (Trim(line) == BVHKeyMotion)
		{
			LoadMotion(file);
		}
	}

	return 1;
}

int	BVHResource::LoadMotion(std::fstream& file)
{
	std::string line;

	while(file.good())
	{
		file >> line;

		line = Trim(line);

		if (line == BVHKeyFrameCount)
		{
			file >> mMotion.FrameCount;
		}
		else if (line == BVHKeyFrame)
		{
			float frameTime;
			file >> line >> frameTime;	// Frame Time: 

			int frameCount		= mMotion.FrameCount;
			int channelCount	= mMotion.ChannelCount;
		
			mMotion.Data = new float[frameCount * channelCount];
			
			for (int frame = 0; frame < frameCount; frame++)
			{
				for (int channel = 0; channel < channelCount; channel++)
				{
					float x;
					std::stringstream ss;
					file >> line;
					ss << line;
					ss >> x;

					mMotion.Data[frame * channelCount + channel] = x;
				}
			}
		}
	}

	return 1;
}