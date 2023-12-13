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
  [[vk::location(3)]] float3 Tangent : TEXCOORD1;
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
  [[vk::location(3)]] float3 Tangent : TEXCOORD1;
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

  float3x3 m3Model = consts.modelMatrix;
  output.Normal = normalize(mul(m3Model, input.Normal));
  output.Tangent = normalize(mul(m3Model, input.Tangent));
  return output;
}

struct PSInput {
  [[vk::location(0)]] float3 WorldPos : POSITION0;
  [[vk::location(1)]] float2 TexCoord : TEXCOORD0;
  [[vk::location(2)]] float3 Normal : NORMAL0;
  [[vk::location(3)]] float3 Tangent : TEXCOORD1;
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
  sampledNormal = sampledNormal * 2.0 - 1.0;

  // Calculate normal in tangent space
  float3 Normal = normalize(input.Normal);
  float3 Tangent = normalize(input.Tangent);
  // Tangent = Tangent - mul(dot(Tangent, Normal), Normal); // Gram–Schmidt
  float3 Bitangent = cross(Normal, Tangent);
  float3x3 TBN = float3x3(Tangent, Bitangent, Normal);
  float3 tnorm = mul(TBN, sampledNormal);
  output.Normal = float4(tnorm, 1.0);

  output.Albedo =
      bindlessTextures[material[consts.materialIndex].albedoTexIndex].Sample(
          bindlessSamplers[material[consts.materialIndex].albedoTexIndex],
          input.TexCoord);

  return output;
}