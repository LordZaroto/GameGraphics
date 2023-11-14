cbuffer ExternalData : register(b0)
{
    matrix view;
    matrix projection;
}

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
    float3 tangent : TANGENT;
};

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

VertexToPixel main(VertexShaderInput input)
{
	// Set up output struct
    VertexToPixel output;
	
    //Create a view matrix with no translation
    matrix viewNoTranslation = view;
    viewNoTranslation._14 = 0;
    viewNoTranslation._24 = 0;
    viewNoTranslation._34 = 0;
    
    matrix vp = mul(projection, viewNoTranslation);
    output.screenPosition = mul(vp, float4(input.localPosition, 1.0f));
    output.screenPosition.z = output.screenPosition.w;
    
    output.sampleDir = input.localPosition;
    
    return output;
}