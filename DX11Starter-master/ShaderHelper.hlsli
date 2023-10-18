#ifndef __GGP_SHADER_INCLUDES__
#define __GGP_SHADER_INCLUDES__
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINTL 1
#define LIGHT_TYPE_SPOT 2
#define MAX_SPECULAR_EXPONENT 256.0f;

struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float3 localPosition : POSITION; // XYZ position
    float3 normal : NORMAL; //Normals
    float2 uv : TEXCOORD; //UVs
};

struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float4 screenPosition : SV_POSITION;
    float2 uv : TEXCOORD; //UVs
    float3 normal : NORMAL;
    float3 worldPosition : POSITION;
};

struct Light
{
    int type;
    float3 direction;
    float range;
    float3 position;
    float intensity;
    float3 color;
    float spotFalloff;
    float3 padding;
};

float3 palette(float t)
{
    float3 a = float3(0.558, 0.648, -0.072);
    float3 b = float3(-0.706, 0.280, -0.830);
    float3 c = float3(0.680, 0.680, 0.288);
    float3 d = float3(1.217, 1.887, 0.358);
    
    return a + b * cos(6.28318 * (c * t + d));
};

#endif