#include "Camera.h"

using namespace Rig3D;

Camera::Camera()
{
}

Camera::~Camera()
{
}

mat4f Camera::GetProjectionMatrix()
{
	return mProjection;
}

mat4f Camera::GetViewMatrix()
{
	if (mTransform.IsDirty())
	{
		mat4f worldMatrix = mTransform.GetWorldMatrix();
		mView = mat4f::lookAtLH(mTransform.GetPosition() + mTransform.GetForward(), mTransform.GetPosition(), vec3f(0.0f, 1.0f, 0.0f));
	}

	return mView;
}

