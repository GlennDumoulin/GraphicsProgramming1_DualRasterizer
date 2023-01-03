//-------------------------
//	Globals
//-------------------------
static const float gPI = 3.1415926535f;
static const float gLightIntensity = 7.f;
static const float gShininess = 25.f;
static const float gKD = 1.f;

static const float3 gLightDirection = { .577f, -.577f, .577f };
static const float3 gAmbientColor = { .025f, .025f, .025f };

float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorld : WORLD;
float4x4 gViewInverse : VIEWINVERSE;

Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;

SamplerState gSampler : Sampler;

RasterizerState gRasterizer : Rasterizer;

BlendState gBlendState
{
	BlendEnable[0] = false;
};

DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = all;
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
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 WorldPosition : COLOR;
	float2 UV : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
};

//-------------------------
//	Helper Functions
//-------------------------
float3 CalculateNormal(VS_OUTPUT input)
{
	const float3 binormal = cross(input.Normal, input.Tangent);
	const float3x3 tangentSpaceAxis = { input.Tangent, normalize(binormal), input.Normal };

	const float4 normalColor = gNormalMap.Sample(gSampler, input.UV);
	float3 sampledNormal = normalColor.rgb;

	sampledNormal = 2 * sampledNormal - float3(1.f, 1.f, 1.f);
	sampledNormal = mul(sampledNormal, tangentSpaceAxis);

	return sampledNormal;
};

float4 CalculateSpecularColor(VS_OUTPUT input, float3 sampledNormal)
{
	const float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInverse[3].xyz);

	const float3 reflectVector = reflect(gLightDirection, sampledNormal);
	const float reflectAngle = saturate(dot(reflectVector, -viewDirection));

	const float4 glossinessColor = gGlossinessMap.Sample(gSampler, input.UV);
	const float glossinessExponent = glossinessColor.r * gShininess;

	const float phongValue = pow(reflectAngle, glossinessExponent);

	return gSpecularMap.Sample(gSampler, input.UV) * phongValue;
};

float3 ShadePixel(VS_OUTPUT input)
{
	//Calculate the normal
	float3 sampledNormal = input.Normal;

	sampledNormal = CalculateNormal(input);

	sampledNormal = normalize(sampledNormal);

	//Calculate the observed area
	const float observedArea = saturate(dot(sampledNormal, -gLightDirection));

	//Calculate the pixel color
	float4 diffuseColor = gDiffuseMap.Sample(gSampler, input.UV);
	diffuseColor = (diffuseColor * gKD / gPI) * gLightIntensity;

	const float4 specularColor = CalculateSpecularColor(input, sampledNormal);

	const float3 finalColor = (diffuseColor.rgb * observedArea) + specularColor.rgb;

	return finalColor + gAmbientColor;
};

//-------------------------
//	Vertex Shader
//-------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	
	output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
	output.WorldPosition = mul(float4(input.Position, 1.f), gWorld);
	output.UV = input.UV;
	output.Normal = mul(normalize(input.Normal), (float3x3)gWorld);
	output.Tangent = mul(normalize(input.Tangent), (float3x3)gWorld);
	
	return output;
}

//-------------------------
//	Pixel Shader
//-------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
	const float3 pixelColor = ShadePixel(input);

	return float4(pixelColor, 1.f);
}

//-------------------------
//	Technique
//-------------------------
technique11 DefaultTechnique
{
	pass P0
	{
		SetRasterizerState(gRasterizer);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
