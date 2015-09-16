#pragma once
#include "Rig3D\Common\Transform.h"

#ifdef _WINDLL
#define RIG3D __declspec(dllexport)
#else
#define RIG3D __declspec(dllimport)
#endif

namespace Rig3D
{
	class RIG3D Camera
	{
	public:
		Transform	mTransform;

		Camera();
		~Camera();
		
		mat4f GetProjectionMatrix(float fovy, float aspectRatio, float zNear, float zFar);
		mat4f GetViewMatrix();

	private:  // Not used yet
		mat4f		mProjection;
		mat4f		mView;
	};

}
