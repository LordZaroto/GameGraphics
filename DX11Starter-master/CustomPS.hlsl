#include "ShaderHelper.hlsli"

cbuffer ExternalData : register(b0)
{
    float4 colorTint;
}

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
    float2 xy = input.uv * 2.0 - 1.0;
    float center = xy;
    float3 finalColor = float3(0.0, 0.0, 0.0);
    
    for (float i = 0.0; i < 3.0; i++)
    {
        xy = frac(xy * 1.5) - 0.5;
    
        float d = length(xy);
        
        float3 color = palette(center);
    
        d = sin(d * 8.0) / 8.0;
        d = abs(d);
    
        d = pow(0.02 / d, 1.9);
    
        finalColor += color * d;
    }
	
    return float4(finalColor, 1.0); //float4(input.uv, 0, 1)
}