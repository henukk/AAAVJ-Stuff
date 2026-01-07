cbuffer PerFrame : register(b1)
{
    float3 L;
    float pad0;
    float3 Lc;
    float pad1;
    float3 Ac;
    float pad2;
    float3 viewPos;
    float pad3;
};

struct PBRPhongMaterialData
{
    float3 diffuseColour;
    uint hasDiffuseTex;

    float3 specularColour;
    float shininess;
};

cbuffer PerInstance : register(b2)
{
    float4x4 modelMat;
    float4x4 normalMat;
    PBRPhongMaterialData material;
};

Texture2D diffuseTex : register(t0);
SamplerState samp : register(s0);

struct VSOutput
{
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float4 position : SV_POSITION;
};

float4 exercise7PS(VSOutput input) : SV_TARGET
{
    float3 N = normalize(input.normal);
    float3 Ldir = normalize(-L);
    float3 V = normalize(viewPos - input.worldPos);

    float NdotL = max(dot(N, Ldir), 0.0f);

    float3 R = reflect(-Ldir, N);
    float spec = pow(max(dot(R, V), 0.0f), material.shininess);

    float3 baseRgb = material.diffuseColour;
    float alpha = 1.0f;

    if (material.hasDiffuseTex != 0)
    {
        float4 tex = diffuseTex.Sample(samp, input.texCoord);
        baseRgb *= tex.rgb;
        alpha = tex.a;
    }

    float3 ambient = Ac * baseRgb;
    float3 diffuse = NdotL * Lc * baseRgb;
    float3 specular = material.specularColour * spec * Lc;

    return float4(ambient + diffuse + specular, alpha);
}