#define BINDLESS_SPACE space0
#define SCENE_SPACE space1
#define MATERIAL_SPACE space2

Texture2D bindlessTextures[] : register(t0, BINDLESS_SPACE);
SamplerState bindlessSamplers[] : register(s0, BINDLESS_SPACE);

struct SceneUBO {
  float4x4 projMatrix;
  float4x4 viewMatrix;
};
cbuffer Scene : register(b0, SCENE_SPACE) { SceneUBO scene; };

struct MaterialUBO {
  float4 albedoColor;
  uint albedoTexIndex;
  uint normalTexIndex;
  float2 padding;
};
cbuffer Material : register(b0, MATERIAL_SPACE) { MaterialUBO material[32]; };

struct VSInput {
  [[vk::location(0)]] float3 Position : POSITION0;
  [[vk::location(1)]] float2 TexCoord : TEXCOORD0;
  [[vk::location(2)]] float3 Normal : NORMAL0;
  [[vk::location(3)]] float3 Tangent : TANGENT0;
};

struct PushConsts {
  float4x4 modelMatrix;
  uint materialIndex;
};
[[vk::push_constant]] PushConsts consts;

struct VSOutput {
  float4 FragPos : SV_POSITION;
  [[vk::location(0)]] float3 WorldPos : POSITION0;
  [[vk::location(1)]] float2 TexCoord : TEXCOORD0;
  [[vk::location(2)]] float3 Normal : NORMAL0;
  [[vk::location(3)]] float3 Tangent : TANGENT0;
};

VSOutput VSMain(VSInput input) {
  VSOutput output;

  output.FragPos =
      mul(scene.projMatrix,
          mul(scene.viewMatrix,
              mul(consts.modelMatrix, float4(input.Position.xyz, 1.0))));

  output.WorldPos =
      mul(consts.modelMatrix, float4(input.Position.xyz, 1.0)).xyz;
  output.TexCoord = input.TexCoord;
  output.Normal = normalize(input.Normal);
  output.Tangent = normalize(input.Tangent);
  return output;
}

struct PSInput {
  [[vk::location(0)]] float3 WorldPos : POSITION0;
  [[vk::location(1)]] float2 TexCoord : TEXCOORD0;
  [[vk::location(2)]] float3 Normal : NORMAL0;
  [[vk::location(3)]] float3 Tangent : TANGENT0;
};

struct PSOutput {
  float4 Position : SV_TARGET0;
  float4 Normal : SV_TARGET1;
  float4 Albedo : SV_TARGET2;
};

PSOutput PSMain(PSInput input) {
  PSOutput output;

  output.Position = float4(input.WorldPos, 1.0);

  float3 sampledNormal =
      bindlessTextures[material[consts.materialIndex].normalTexIndex]
          .Sample(
              bindlessSamplers[material[consts.materialIndex].normalTexIndex],
              input.TexCoord)
          .xyz;
  // Calculate normal in tangent space
  float3 N = normalize(input.Normal);
  float3 T = normalize(input.Tangent);
  float3 B = cross(N, T);
  float3x3 TBN = float3x3(T, B, N);
  float3 tnorm =
      mul(normalize(sampledNormal * 2.0 - float3(1.0, 1.0, 1.0)), TBN);
  output.Normal = float4(tnorm, 1.0);
  //   output.Normal = float4(input.Normal, 1.0);

  output.Albedo =
      bindlessTextures[material[consts.materialIndex].albedoTexIndex].Sample(
          bindlessSamplers[material[consts.materialIndex].albedoTexIndex],
          input.TexCoord);

  return output;
}