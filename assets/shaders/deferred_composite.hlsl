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

Texture2D positionTexture : register(t0, space0);
SamplerState positionSampler : register(s0, space0);

Texture2D normalTexture : register(t1, space0);
SamplerState normalSampler : register(s1, space0);

Texture2D albedoTexture : register(t2, space0);
SamplerState albedoSampler : register(s2, space0);

struct PSInput {
  [[vk::location(0)]] float2 TexCoord : TEXCOORD0;
};

struct PSOutput {
  float4 FragColor : SV_TARGET0;
};

static float gAmbient = 0.1;
struct Light {
  float4 Position;
  float3 Color;
  float Radius;
};
Light MakeLight(float4 position, float3 color, float radius) {
  Light light;
  light.Position = position;
  light.Color = color;
  light.Radius = radius;
  return light;
}
static Light gLight = MakeLight(float4(-3, 3, 3, 0), float3(1, 1, 1), 2.0f);

PSOutput PSMain(PSInput input) {
  PSOutput output;

  float3 fragPos = positionTexture.Sample(positionSampler, input.TexCoord).rgb;
  float3 normal = normalTexture.Sample(normalSampler, input.TexCoord).rgb;
  float4 albedo = albedoTexture.Sample(albedoSampler, input.TexCoord);

  float3 fragColor = albedo.rgb * gAmbient;

  // For each light
  {
    // Vector to light
    float3 L = normalize(gLight.Position.xyz - fragPos);
    // Distance from light to fragment position
    float dist = length(L);

    // Attentuation
    float atten = gLight.Radius / (pow(dist, 2.0) + 1.0);

    // Diffuse part
    float3 N = normalize(normal);
    float NdotL = max(0.0, dot(N, L));
    float3 diff = gLight.Color * albedo.rgb * NdotL * atten;

    fragColor += diff;
  }

  output.FragColor = float4(fragColor, 1.0);

  return output;
}