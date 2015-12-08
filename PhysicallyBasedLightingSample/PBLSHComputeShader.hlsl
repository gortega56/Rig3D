

struct Pixel
{
	float4 color;
};

Texture2D textureCube : register(t0);


[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
}