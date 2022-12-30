//-------------------------
//	Globals
//-------------------------
float4x4 gWorldViewProj : WorldViewProjection;

Texture2D gDiffuseMap : DiffuseMap;

SamplerState gSampler : Sampler;

RasterizerState gRasterizerState
{
	CullMode = none;
	FrontCounterClockwise = false; //default
};

BlendState gBlendState
{
	BlendEnable[0] = true;
	SrcBlend = src_alpha;
	DestBlend = inv_src_alpha;
	BlendOp = add;
	SrcBlendAlpha = zero;
	DestBlendAlpha = zero;
	BlendOpAlpha = add;
	RenderTargetWriteMask[0] = 0x0F;
};

DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = zero;
	DepthFunc = less;
	StencilEnable = false;
};

//-------------------------
//	Input/Output Structs
//-------------------------
struct VS_INPUT
{
	float3 Position : POSITION;
	float2 UV : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD;
};

//-------------------------
//	Helper Functions
//-------------------------
float4 ShadePixel(VS_OUTPUT input)
{
	const float4 diffuseColor = gDiffuseMap.Sample(gSampler, input.UV);

	return diffuseColor;
};

//-------------------------
//	Vertex Shader
//-------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	
	output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
	output.UV = input.UV;
	
	return output;
}

//-------------------------
//	Pixel Shader
//-------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
	const float4 pixelColor = ShadePixel(input);

	return pixelColor;
}

//-------------------------
//	Technique
//-------------------------
technique11 DefaultTechnique
{
	pass P0
	{
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
