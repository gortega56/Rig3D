struct SampleVertex
{
	float3 mPosition	: POSITION;
	float3 mColor		: COLOR;
};

struct SamplePixel
{
	float4 mPositionH	: SV_POSITION;
	float2 mColor		: COLOR;		// Keep this as color for right now
};

SamplePixel main(SampleVertex vertex)
{
	SamplePixel pixel;
	pixel.mPositionH = float4(vertex.mPosition, 1.0f);
	pixel.mColor = vertex.mColor.xy;

	return pixel;
}