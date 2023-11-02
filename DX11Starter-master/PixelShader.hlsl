#include "ShaderHelper.hlsli"

cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float3 cameraPos;
    float roughness;
    float3 ambient;
    float2 scale;
    float2 offset;
    //Light lights[5];
    Light directionalLight;
    Light directionalLight2;
    Light directionalLight3;
    Light pointLight;
    Light pointLight2;
}

//Need at least 1 sampler for textures
SamplerState BasicSampler : register(s0);

//Textures
Texture2D DiffuseTexture : register(t0);
Texture2D NormalMap : register(t1);

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
    //Scale
    float2 scaleCenter = float2(0.5f, 0.5f);
    input.uv = (input.uv - scaleCenter) * scale + scaleCenter;
    
    //Offset
    input.uv += offset;
    
    //Normal
    float3 unpackedNormal = normalize(NormalMap.Sample(BasicSampler, input.uv).rgb * 2 - 1);
    float3 N = normalize(input.normal);
    float3 T = normalize(input.tangent);
    T = normalize(T - N * dot(T, N));
    float3 B = cross(T, N);
    float3x3 TBN = float3x3(T, B, N);
    input.normal = mul(unpackedNormal, TBN);
    
    float3 diffuse = DiffuseTexture.Sample(BasicSampler, input.uv);
    float3 V = normalize(cameraPos - input.worldPosition);
    float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
    float3 light = float3(0, 0, 0);
    
    /*for (int i = 0; i < 5; i++)
    {
        [branch] switch (lights[i].type)
        {
            case LIGHT_TYPE_DIRECTIONAL:
                light += DiffuseSpecCalc(lights[i], input.normal, colorTint, ambient, specExponent, V, lights[i].direction);
                break;
            
            case LIGHT_TYPE_POINT:
                float3 direction = normalize(input.worldPosition - lights[i].position);
                light += DiffuseSpecCalc(lights[i], input.normal, colorTint, ambient, specExponent, V, direction)
                         * Attenuate(lights[i], input.worldPosition);
                break;
            
            case LIGHT_TYPE_SPOT:
                light += 0;
                break;
        }
    }*/
    
    //Directional Light
    light += DiffuseSpecCalc(directionalLight, input.normal, colorTint, specExponent, V, directionalLight.direction);
    light += DiffuseSpecCalc(directionalLight2, input.normal, colorTint, specExponent, V, directionalLight2.direction);
    light += DiffuseSpecCalc(directionalLight3, input.normal, colorTint, specExponent, V, directionalLight3.direction);
    
    //Point Light
    float3 direction = normalize(input.worldPosition - pointLight.position);
    light += DiffuseSpecCalc(pointLight, input.normal, colorTint, specExponent, V, direction)
             * Attenuate(pointLight, input.worldPosition);
    
    direction = normalize(input.worldPosition - pointLight2.position);
    light += DiffuseSpecCalc(pointLight2, input.normal, colorTint, specExponent, V, direction)
             * Attenuate(pointLight, input.worldPosition);
    
    light *= colorTint.xyz;
    light += ambient;
    
    //return float4(roughness.rrr, 1);
    //return float4(input.uv, 0, 1);
    //return float4(input.normal, 1.0);
    return float4(light * diffuse, 1);
}