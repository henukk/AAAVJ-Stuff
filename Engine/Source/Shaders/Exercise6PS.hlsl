cbuffer PerFrame : register(b1) {
    float3 L;
    float pad0;
    float3 Lc;
    float pad1;
    float3 Ac;
    float pad2;
    float3 viewPos;
    float pad3;
};

struct PhongMaterialData {
    float4 diffuseColour;
    float Kd;
    float Ks;
    float shininess;
    bool hasDiffuseTex;
};

cbuffer PerInstance : register(b2) {
    float4x4 modelMat;
    float4x4 normalMat;
    PhongMaterialData material;
};

Texture2D diffuseTex : register(t0);
SamplerState samp : register(s0);

struct VSOutput {
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float4 position : SV_POSITION;
};

float4 exercise6PS(VSOutput input) : SV_TARGET
{
    float3 N = normalize(input.normal);
    float3 Ldir = normalize(-L);
    float3 V = normalize(viewPos - input.worldPos);

    // Diffuse
    float NdotL = max(dot(N, Ldir), 0.0f);

    // Specular
    float3 R = reflect(-Ldir, N);
    float spec = pow(max(dot(R, V), 0.0f), material.shininess);

    float4 baseColor = material.diffuseColour;
    if (material.hasDiffuseTex)
    {
        baseColor *= diffuseTex.Sample(samp, input.texCoord);
    }

    float3 ambient = Ac * baseColor.rgb;
    float3 diffuse = material.Kd * NdotL * Lc * baseColor.rgb;
    float3 specular = material.Ks * spec * Lc;

    return float4(ambient + diffuse + specular, baseColor.a);
}
