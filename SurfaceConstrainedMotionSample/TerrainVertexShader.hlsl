struct Vertex
{
	float3		position		: POSITION;
	float3		normal			: NORMAL;
	float2		uv				: TEXCOORD;
	float4x4	world			: WORLD;
	uint		instanceID		: SV_InstanceID;
};

struct Pixel
{
	float4 position		: SV_POSITION;
	float3 positionT	: POSITIONT;
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
};

// Offset by half extents of terrain
// Normalize position with inverted Y
// Divide by number of patches per dimension

cbuffer transform : register(b0)
{
	matrix view;
	matrix projection;
}

cbuffer terrain : register (b1)
{
	float width;
	float depth;
	float widthPatchCount;
	float depthPatchCount;
}

Texture2D heightTexture : register(t0);
SamplerState samplerState: register(s0);

Pixel main(Vertex vertex)
{
	float3 position = float3(vertex.world[3][0], vertex.world[3][1], vertex.world[3][2]);
	position.x -= width;
	position.z -= depth;
	position = normalize(position);
	
	float2 uv = float2(((position.x + 1.0f) * 0.5f) / widthPatchCount, ((1.0f - position.z) * 0.5f) / depthPatchCount) * vertex.uv;

	uint xid = vertex.instanceID / uint(widthPatchCount);
	uint zid = vertex.instanceID % uint(widthPatchCount);
	float2 id = float2(zid, xid);

	float x = (vertex.uv.x / widthPatchCount) + id.x * 1 / widthPatchCount;
	float z = (vertex.uv.y / depthPatchCount) + id.y * 1 / depthPatchCount;

	float y = heightTexture.SampleLevel(samplerState, float2(x, z), 0).r * 3.0f;
	float4 vertexPosition = float4(vertex.position.x, y, vertex.position.z, 1.0f);

	matrix clip = mul(mul(vertex.world, view), projection);

	Pixel pixel;
	pixel.position = mul(vertexPosition, clip);

	float c = widthPatchCount * depthPatchCount;
	pixel.positionT = mul(vertexPosition, vertex.world).xyz;
	pixel.normal = mul(float4(vertex.normal, 0.0f), vertex.world).xyz;
	pixel.uv = vertex.uv;

	return pixel;
}