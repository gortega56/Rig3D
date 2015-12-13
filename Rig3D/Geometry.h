#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>
#include <math.h>

namespace Rig3D
{
	namespace Geometry
	{
		template <class Vertex, class Index>
		void Plane(std::vector<Vertex>& vertices, std::vector<Index>& indices, float width, float depth, uint32_t vertexWidth, uint32_t vertexDepth)
		{
			vertices.reserve(sizeof(Vertex) * vertexWidth * vertexDepth);
			indices.reserve(sizeof(Index) * (vertexWidth - 1) * (vertexDepth - 1) * 6);

			float widthStep = width / static_cast<float>(vertexWidth - 1);
			float depthStep = depth / static_cast<float>(vertexDepth - 1);

			float halfWidth = width * 0.5f;
			float halfDepth = depth * 0.5f;

			for (uint32_t z = 0; z < vertexDepth; z++)
			{
				for (uint32_t x = 0; x < vertexWidth; x++)
				{
					Vertex v0;
					v0.Position = { x * widthStep - halfWidth, 0.0f, z * depthStep - halfDepth };
					v0.Normal	= { 0.0f, 1.0f, 0.0f };
					v0.UV = { static_cast<float>(x) / (vertexWidth - 1), 1.0f - (static_cast<float>(z) / (vertexDepth - 1)) };
					vertices.push_back(v0);
				}
			}

			for (uint32_t z = 0; z < vertexDepth - 1; z++)
			{
				for (uint32_t x = 0; x < vertexWidth - 1; x++)
				{
					indices.push_back((z * vertexWidth) + x);
					indices.push_back(((z + 1) * vertexWidth) + x);
					indices.push_back(((z + 1) * vertexWidth) + x + 1);
					indices.push_back(((z + 1) * vertexWidth) + x + 1);
					indices.push_back((z * vertexWidth) + x + 1);
					indices.push_back((z * vertexWidth) + x);
				}
			}
		}

		template <class Vertex, class Index>
		static void Sphere(std::vector<Vertex>& vertices, std::vector<Index>& indices, uint32_t azimuthSubdivisions, uint32_t polarSubdivisions, float radius)
		{
			vertices.reserve(sizeof(Vertex) * azimuthSubdivisions * polarSubdivisions);
			indices.reserve(sizeof(Index) * (azimuthSubdivisions - 1) * (polarSubdivisions - 1) * 6);

			float pi = static_cast<float>(M_PI);

			float phiStep = (2.0f * pi) / static_cast<float>(azimuthSubdivisions);
			float thetaStep = pi / static_cast<float>(polarSubdivisions);
			
			for (float phi = 0.0f; phi <= (2.0f * pi); phi += phiStep)
			{
				for (float theta = 0.0f; theta <= pi; theta += thetaStep)
				{
					float sinTheta = sin(theta);
					float cosTheta = cos(theta);
					float sinPhi = sin(phi);
					float cosPhi = cos(phi);

					vec3f normal = { sinTheta * cosPhi, sinTheta * sinPhi, cosTheta };

					Vertex vertex;
					vertex.Position = radius * normal;
					vertex.Normal = normal;
					vertex.UV = { 0.5f + (atan2(normal.z, vertex.Normal.x) / (2.0f * pi)), 0.5f - (asin(normal.y) / pi) };
					vertices.push_back(vertex);
				}
			}

			for (uint32_t azimuth = 0; azimuth < azimuthSubdivisions - 1; azimuth++)
			{
				for (uint32_t polar = 0; polar < polarSubdivisions - 1; polar++)
				{
					uint32_t i0 = (azimuth * (polarSubdivisions - 1)) + polar;
					uint32_t i1 = i0 + polarSubdivisions + 1;

					indices.push_back(i0);
					indices.push_back(i0 + 1);
					indices.push_back(i1);
					indices.push_back(i1);
					indices.push_back(i0 + 1);
					indices.push_back(i1 + 1);
				}
			}
		}
	}
}