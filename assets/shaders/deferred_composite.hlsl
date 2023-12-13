#define GBUFFER_SPACE space0
#define LIGHTING_SPACE space1

#define MAX_LIGHTS 16

struct VSOutput {
  float4 FragPos : SV_POSITION;
  [[vk::location(0)]] float2 TexCoord : TEXCOORD0;
};

VSOutput VSMain(uint VertexId : SV_VERTEXID) {
  VSOutput output;

  output.TexCoord = float2((VertexId << 1) & 2, VertexId & 2);
  output.FragPos = float4(output.TexCoord * 2.0 - 1.0, 0.0, 1.0);

  return output;
}

Texture2D positionTexture : register(t0, GBUFFER_SPACE);
SamplerState positionSampler : register(s0, GBUFFER_SPACE);

Texture2D normalTexture : register(t1, GBUFFER_SPACE);
SamplerState normalSampler : register(s1, GBUFFER_SPACE);

Texture2D albedoTexture : register(t2, GBUFFER_SPACE);
SamplerState albedoSampler : register(s2, GBUFFER_SPACE);

struct Light {
  float4 position;
  float4 direction;
  float4 color; // XYZ=Color, W=Intensity
};
struct LightingUBO {
  float4 ambientLight;
  Light lights[MAX_LIGHTS];
};
cbuffer Lighting : register(b0, LIGHTING_SPACE) { LightingUBO lighting; };

struct PSInput {
  [[vk::location(0)]] float2 TexCoord : TEXCOORD0;
};

struct PSOutput {
  float4 FragColor : SV_TARGET0;
};

float3 CalculateDirLight(float3 normal, float3 albedo, float3 direction,
                         float3 color, float intensity) {
  float3 lightColor = color.rgb * intensity;
  float3 lightDir = normalize(direction);

  // Diffuse
  float3 norm = normalize(normal);
  float3 diff = max(dot(norm, lightDir), 0.0);

  return lightColor * diff * albedo.rgb;
}

PSOutput PSMain(PSInput input) {
  PSOutput output;

  float3 fragPos = positionTexture.Sample(positionSampler, input.TexCoord).rgb;
  float3 normal = normalTexture.Sample(normalSampler, input.TexCoord).rgb;
  float4 albedo = albedoTexture.Sample(albedoSampler, input.TexCoord);

  float3 fragColor =
      albedo.rgb * (lighting.ambientLight.rgb * lighting.ambientLight.a);

  for (uint i = 0; i < MAX_LIGHTS; ++i) {
    float3 lightColor = float3(0, 0, 0);
    if (lighting.lights[i].position.w == 1.0)
      lightColor += CalculateDirLight(
          normal, albedo.rgb, lighting.lights[i].direction.xyz,
          lighting.lights[i].color.rgb, lighting.lights[i].color.a);

    fragColor += lightColor;
  }

  output.FragColor = float4(fragColor, 1.0);

  return output;
}