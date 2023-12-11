#define SCENE_SPACE space0

struct SceneUBO
{
	float4x4 projMatrix;
	float4x4 viewMatrix;
};
cbuffer Scene : register(b0, SCENE_SPACE)
{
	SceneUBO scene;
};

struct VSInput
{
	[[vk::location(0)]] float3 Position : POSITION0;
	[[vk::location(1)]] float2 TexCoord : TEXCOORD0;
	[[vk::location(2)]] float3 Normal : NORMAL0;
	[[vk::location(3)]] float3 Tangent : TANGENT0;
};

struct PushConsts
{
	float4x4 modelMatrix;
};
[[vk::push_constant]] PushConsts consts;

struct VSOutput
{
	float4 FragPos : SV_POSITION;
	[[vk::location(0)]] float3 WorldPos : POSITION0;
	[[vk::location(1)]] float2 TexCoord : TEXCOORD0;
	[[vk::location(2)]] float3 Normal : NORMAL0;
	[[vk::location(3)]] float3 Tangent : TANGENT0;
};

VSOutput VSMain(VSInput input)
{
	VSOutput output;

	output.FragPos = mul(scene.projMatrix, mul(scene.viewMatrix, mul(consts.modelMatrix, float4(input.Position.xyz, 1.0))));

	output.WorldPos = mul(consts.modelMatrix, float4(input.Position.xyz, 1.0)).xyz;
	output.TexCoord = input.TexCoord;
	output.Normal = normalize(input.Normal);
	output.Tangent = normalize(input.Tangent);
	return output;
}

struct PSInput
{
	[[vk::location(0)]] float3 WorldPos : POSITION0;
	[[vk::location(1)]] float2 TexCoord : TEXCOORD0;
	[[vk::location(2)]] float3 Normal : NORMAL0;
	[[vk::location(3)]] float3 Tangent : TANGENT0;
};

struct PSOutput
{
	float4 FragColor : SV_TARGET;
};

PSOutput PSMain(PSInput input)
{
	PSOutput output;

	// Calculate normal in tangent space
	/* float3 N = normalize(input.Normal);
	float3 T = normalize(input.Tangent);
	float3 B = cross(N, T);
	float3x3 TBN = float3x3(T, B, N);
	float3 tnorm = mul(
		normalize(textureNormalMap.Sample(samplerNormalMap, input.UV).xyz * 2.0 -
				  float3(1.0, 1.0, 1.0)),
		TBN);
	output.Normal = float4(tnorm, 1.0); */
	// output.Normal = float4(input.Normal, 1.0);

	// output.Albedo = textureColor.Sample(samplerColor, input.UV);
	output.FragColor = float4(1, 1, 1, 1);

	return output;
}