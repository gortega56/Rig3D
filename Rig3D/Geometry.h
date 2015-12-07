#pragma once
#include <vector>

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
	}
}