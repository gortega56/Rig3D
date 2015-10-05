#include "Transform.h"
#include "GraphicsMath\Quaternion.hpp"

using namespace Rig3D;

Transform::Transform() : 
mPosition(0.0, 0.0, 0.0), 
mRotation(0.0, 0.0, 0.0), 
mScale(1.0, 1.0, 1.0),
mForward(0.0, 0.0, 1.0), 
mRight(1.0, 0.0, 0.0), 
mUp(0.0, 1.0, 0.0)
{
}

Transform::~Transform()
{
}

void Transform::MoveForward()
{
	mPosition.x += mForward.x;
	mPosition.y += mForward.y;
	mPosition.z += mForward.z;
}

void Transform::MoveBackward()
{
	mPosition.x -= mForward.x;
	mPosition.y -= mForward.y;
	mPosition.z -= mForward.z;
}

void Transform::MoveLeft()
{
	mPosition.x -= mRight.x;
	mPosition.y -= mRight.y;
	mPosition.z -= mRight.z;
}

void Transform::MoveRight()
{
	mPosition.x += mRight.x;
	mPosition.y += mRight.y;
	mPosition.z += mRight.z;
}

void Transform::RotatePitch(float pitch)
{
	mRotation.x += pitch;
}

void Transform::RotateYaw(float yaw)
{
	mRotation.y += yaw;
}

void Transform::RotateRoll(float roll)
{
	mRotation.z += roll;
}

mat4f Transform::GetWorldMatrix()
{
	mat4f translation	= mat4f::translate(mPosition);
	mat4f rotation = quatf::rollPitchYaw(mRotation.z, mRotation.x, mRotation.y).toMatrix4();
	mat4f scale			= mat4f::scale(mScale);
	return scale * rotation * translation;
}

mat3f Transform::GetRotationMatrix()
{
	return quatf::rollPitchYaw(mRotation.z, mRotation.x, mRotation.y).toMatrix3();
}

vec3f Transform::GetForward()
{
	return vec3f(0.0, 0.0, 1.0) * quatf::rollPitchYaw(mRotation.z, mRotation.x, mRotation.y);
}

vec3f Transform::GetUp()
{
	return vec3f(0.0, 1.0, 0.0) * quatf::rollPitchYaw(mRotation.z, mRotation.x, mRotation.y);
}

vec3f Transform::GetRight()
{
	return vec3f(1.0, 0.0, 0.0) * quatf::rollPitchYaw(mRotation.z, mRotation.x, mRotation.y);
}