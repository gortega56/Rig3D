struct Vertex
{
	float3 mPosition	: POSITION;
	float3 mUV			: TEXCOORD;
};

struct Pixel
{
	float4 mPositionH	: SV_POSITION;
	float3 mUV			: TEXCOORD;
};

cbuffer transform : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
}

Pixel main(Vertex vertex)
{
	matrix clip = mul(mul(world, view), projection);

	Pixel pixel;
	pixel.mPositionH = mul(float4(vertex.mPosition, 1.0f), clip);
	pixel.mUV = vertex.mUV;

	return pixel;
}