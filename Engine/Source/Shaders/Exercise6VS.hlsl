// ---------- MVP ----------
cbuffer MVP : register(b0) {
    float4x4 mvp;
};

// ---------- Per-Instance ----------
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
    PhongMaterialData material; // no se usa aquí, pero el layout debe coincidir
};

// ---------- Input ----------
struct VSInput {
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
};

// ---------- Output ----------
struct VSOutput {
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float4 position : SV_POSITION;
};

VSOutput exercise6VS(VSInput input) {
    VSOutput output;

    float4 worldPos = mul(float4(input.position, 1.0f), modelMat);

    output.worldPos = worldPos.xyz;
    output.normal = normalize(mul(input.normal, (float3x3) normalMat));
    output.texCoord = input.texCoord;
    output.position = mul(float4(input.position, 1.0f), mvp);

    return output;
}
