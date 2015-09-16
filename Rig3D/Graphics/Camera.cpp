#include "Camera.h"

using namespace Rig3D;

Camera::Camera()
{
}

Camera::~Camera()
{
}

mat4f Camera::GetProjectionMatrix(float fovy, float aspectRatio, float zNear, float zFar)
{
	return mat4f::perspective(fovy, aspectRatio, zNear, zFar);
}

mat4f Camera::GetViewMatrix()
{
	return mTransform.GetWorldMatrix().inverse();
}