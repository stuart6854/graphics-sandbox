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
  float3 position;
  uint type;    // 0=Directional
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

PSOutput PSMain(PSInput input) {
  PSOutput output;

  float3 fragPos = positionTexture.Sample(positionSampler, input.TexCoord).rgb;
  float3 normal = normalTexture.Sample(normalSampler, input.TexCoord).rgb;
  float4 albedo = albedoTexture.Sample(albedoSampler, input.TexCoord);

  float3 fragColor =
      albedo.rgb * (lighting.ambientLight.rgb * lighting.ambientLight.a);

  // For each light
  for (uint i = 0; i < MAX_LIGHTS; ++i) {
    float3 lightColor =
        lighting.lights[i].color.rgb * lighting.lights[i].color.a;

    // Vector to light
    float3 L = normalize(lighting.lights[i].position.xyz - fragPos);
    // Distance from light to fragment position
    float dist = length(L);

    // Attentuation
    // float atten = lighting.lights[i].Radius / (pow(dist, 2.0) + 1.0);

    // Diffuse part
    float3 N = normalize(normal);
    float NdotL = max(0.0, dot(N, L));
    float3 diff = lightColor * albedo.rgb * NdotL; // * atten;

    fragColor += diff;
  }

  output.FragColor = float4(fragColor, 1.0);

  return output;
}