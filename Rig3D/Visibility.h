#pragma once
#include "GraphicsMath/cgm.h"
#include "Rig3D/Intersection.h"

namespace Rig3D
{
	struct Frustum
	{
		Plane<vec3f> nearPlane;
		Plane<vec3f> farPlane;
		Plane<vec3f> leftPlane;
		Plane<vec3f> rightPlane;
		Plane<vec3f> topPlane;
		Plane<vec3f> bottomPlane;
	};

	inline void ExtractFrustum(Frustum* frustum, mat4f viewProjectionMatrix)
	{
		
	}

	inline void Cull(const Frustum& frustum, Sphere<vec3f>* spheres, const uint32_t& count)
	{
		
	}
}