#pragma once
#include "GraphicsMath\cgm.h"

#ifdef _WINDLL
#define RIG3D __declspec(dllexport)
#else
#define RIG3D __declspec(dllimport)
#endif

namespace Rig3D
{
	class RIG3D Transform
	{
	public:
		vec3f	mPosition;
		vec3f	mRotation;
		vec3f	mScale;

		Transform();
		~Transform();

		void MoveForward();
		void MoveBackward();
		void MoveRight();
		void MoveLeft();
		void RotatePitch(float pitch);
		void RotateYaw(float yaw);
		void RotateRoll(float roll);

		mat4f GetWorldMatrix();
		mat3f GetRotationMatrix();
		vec3f GetForward();
		vec3f GetUp();
		vec3f GetRight();

	private: // Not used yet.
		mat4f	mWorldMatrix;
		vec3f	mForward;
		vec3f	mUp;
		vec3f	mRight;
	};
}

