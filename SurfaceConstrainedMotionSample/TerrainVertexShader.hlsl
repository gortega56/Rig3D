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
};

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
	float padding[2];
}

Texture2D heightTexture : register(t0);
SamplerState samplerState: register(s0);

Pixel main(Vertex vertex)
{
	matrix clip = mul(mul(vertex.world, view), projection);
	float2 uvOffset = float2(vertex.uv.x / widthPatchCount, vertex.uv.y / depthPatchCount);
	float y = heightTexture.SampleLevel(samplerState, uvOffset, 0).r;
	float4 vertexPosition = float4(vertex.position.x, y, vertex.position.z, 1.0f);

	Pixel pixel;
	pixel.position = mul(vertexPosition, clip);
	pixel.positionT = mul(vertexPosition, vertex.world).xyz;
	pixel.normal = mul(float4(vertex.normal, 0.0f), vertex.world).xyz;

	return pixel;
}