cbuffer ExternalData : register(b0)
{
    float4 colorTint;
}
// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float4 screenPosition : SV_POSITION;
    float2 uv : TEXCOORD; //UVs
};

float3 palette(float t)
{
    float3 a = float3(0.558, 0.648, -0.072);
    float3 b = float3(-0.706, 0.280, -0.830);
    float3 c = float3(0.680, 0.680, 0.288);
    float3 d = float3(1.217, 1.887, 0.358);
    
    return a + b * cos(6.28318 * (c * t + d));
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