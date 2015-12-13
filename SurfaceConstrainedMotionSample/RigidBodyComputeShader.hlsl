struct RigidBody
{
	float3 position;
	float3 velocity;
	float3 forces;
	float inverseMass;
};

struct Plane
{
	float3 normal;
	float  distance;
};

Texture2D heightTexture : register(t0);
Texture2D normalTexture : register(t1);
SamplerState samplerState : register(s0);

RWStructuredBuffer<RigidBody> rigidBodies : register(u0);

cbuffer terrain : register(b0)
{
	float width;
	float depth;
	float widthPatchCount;
	float depthPatchCount;
};

float CalculatePlaneSphereImpulse(RigidBody rigidBody, Plane plane)
{
	float3 vRel = rigidBody.velocity;
	float numerator = (1.0f + 1.0f) * dot(-vRel, plane.normal);
	float denominator = dot((rigidBody.inverseMass) * plane.normal, plane.normal);
	return numerator / denominator;
}

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	RigidBody rb = rigidBodies[(DTid.x * 1) + DTid.y];
	float u = (rb.position.x + (width * 0.5f)) / width;
	float v = (rb.position.z + (depth * 0.5f)) / depth;

	float2 uv = float2(u, 1.0f - v);
	float3 normal = normalTexture.SampleLevel(samplerState, uv, 0).xyz;
	float height = heightTexture.SampleLevel(samplerState, uv, 0).r * 5.0f;
	float3 planePos = float3(rb.position.x, height, rb.position.z);

	float3x3 TBN = float3x3
		(
			float3(1.0f, 0.0f, 0.0f),
			float3(0.0f, 0.0f, 1.0f),
			float3(0.0f, 1.0f, 0.0f)
		);

	normal = normal * 2.0f - 1.0f;
	normal = mul(normal, TBN);

	Plane plane;
	plane.normal = normalize(normal);
	plane.distance = length(planePos);

	float d = dot(plane.normal, rb.position) - plane.distance;
	float radius = 1.0f;

	// Test for plane negative half space
	if (d <= radius)
	{
		float3 poi = rb.position - (plane.normal * abs(d));
		if (length(poi - planePos) < radius)
		{
			float k = CalculatePlaneSphereImpulse(rb, plane);
			rb.velocity += k * plane.normal * rb.inverseMass;
		}
		
		
		float3 g = float3(0.0f, -0.0000098196f, 0.0f);
		float3 n = plane.normal * dot(g, plane.normal);
		rb.forces += n;
	}

	rigidBodies[(DTid.x * 1) + DTid.y] = rb;
}