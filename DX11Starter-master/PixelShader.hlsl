#include "ShaderHelper.hlsli"

cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float3 cameraPos;
    float roughness;
    float3 ambient;
    float2 scale;
    float2 offset;
    Light directionalLight;
    Light directionalLight2;
    Light directionalLight3;
    Light pointLight;
    Light pointLight2;
}

//Need at least 1 sampler for textures
SamplerState BasicSampler : register(s0);

//Textures
Texture2D Albedo : register(t0);
Texture2D NormalMap : register(t1);
Texture2D RoughnessMap : register(t2);
Texture2D MetalnessMap : register(t3);


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
    
    //PBR parameters
    float3 albedo = pow(Albedo.Sample(BasicSampler, input.uv).rgb, 2.2f);
    float roughness = RoughnessMap.Sample(BasicSampler, input.uv).r;
    float metalness = MetalnessMap.Sample(BasicSampler, input.uv).r;
    float3 specularColor = lerp(F0_NON_METAL, albedo.rgb, metalness);
    float3 toCamera = normalize(cameraPos - input.worldPosition);
    float3 light = float3(0, 0, 0);
    
    //Directional Light
    light += DiffuseSpecCalc(directionalLight, input.normal, directionalLight.direction, toCamera,
                             albedo, roughness, metalness, specularColor);
    light += DiffuseSpecCalc(directionalLight2, input.normal, directionalLight2.direction, toCamera,
                             albedo, roughness, metalness, specularColor);
    light += DiffuseSpecCalc(directionalLight3, input.normal, directionalLight3.direction, toCamera,
                             albedo, roughness, metalness, specularColor);
    
    //Point Light
    float3 direction = normalize(input.worldPosition - pointLight.position);
    light += DiffuseSpecCalc(pointLight, input.normal, direction, toCamera,
             albedo, roughness, metalness, specularColor)
             * Attenuate(pointLight, input.worldPosition);
    
    direction = normalize(input.worldPosition - pointLight2.position);
    light += DiffuseSpecCalc(pointLight2, input.normal, direction, toCamera,
             albedo, roughness, metalness, specularColor)
             * Attenuate(pointLight, input.worldPosition);
    
    return float4(pow(light * albedo, 1.0f / 2.2f), 1);
}