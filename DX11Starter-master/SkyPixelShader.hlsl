TextureCube SkyTexture : register(t0);
SamplerState BasicSampler : register(s0);

struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float4 screenPosition : SV_POSITION;
    float3 sampleDir : DIRECTION;
};

float4 main(VertexToPixel input) : SV_TARGET
{
    return SkyTexture.Sample(BasicSampler, input.sampleDir);
}