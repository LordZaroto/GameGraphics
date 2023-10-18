#include "ShaderHelper.hlsli"

cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float3 cameraPos;
    float roughness;
    float3 ambient;
    Light directionalLight;
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
    input.normal = normalize(input.normal);
    
    float3 diffuse = saturate(dot(input.normal, -directionalLight.direction)) * 
                    (directionalLight.color * directionalLight.intensity * colorTint.xyz) +
                    (ambient * colorTint.xyz);
    
    float V = normalize(directionalLight.position - input.worldPosition);
    float R = reflect(directionalLight.direction, input.normal);
    float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
    
    float spec = 0;
    
    if (specExponent > 0.05f)
    {
        spec = pow(saturate(dot(R, V)), specExponent);
    }
    
    float3 light = colorTint.xyz * diffuse + spec;
    
    //return float4(roughness.rrr, 1);
    //return float4(input.uv, 0, 1);
    //return float4(input.normal, 1.0);
    return float4(light, 1);
}