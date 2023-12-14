cbuffer externalData : register(b0)
{
    int blurRadius;
    float pixelWidth;
    float pixelHeight;
}

struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

Texture2D Pixels : register(t0);
SamplerState ClampSampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
    float4 total = 0;
    int sampleCount = 0;
    
    //Loop through the frame
    for (int x = -blurRadius; x <= blurRadius; x++)
    {
        for (int y = -blurRadius; y <= blurRadius; y++)
        {
            float2 uv = input.uv;
            uv += float2(x * pixelWidth, y * pixelHeight);
            
            total += Pixels.Sample(ClampSampler, uv);
            sampleCount++;
        }
    }
    
    return total / sampleCount;
}