#version 450
struct PBRParameters
{
    vec3 albedo; // 16字节
    float metallic;
    float roughness;
    float ao;
    float emissive;
};

struct PBRTextureFlag
{
    bool useAlbedoMap; // GLSL 中 bool 占 4 字节
    bool useNormalMap;
    bool useMetallicRoughnessMap;
    bool useAOMap;
    bool useEmissiveMap;
};

layout(location = 0) out vec4 outColor;
layout(location = 2) in vec2 inTexCoords;

layout(std140, set = 1, binding = 0) uniform MaterialParams
{
    PBRParameters parameters;
    PBRTextureFlag textureFlag;
}
pbrMaterial;
layout(set = 1, binding = 1) uniform sampler2D AlbedoMap;

void main()
{
    if (pbrMaterial.textureFlag.useAlbedoMap)
    {
        outColor = texture(AlbedoMap, inTexCoords);
    }
    else
    {
        outColor = vec4(pbrMaterial.parameters.albedo, 1.0);
    }
    // outColor = vec4(pbrMaterial.textureFlag.useAlbedoMap, pbrMaterial.textureFlag.useNormalMap,
    //                 pbrMaterial.textureFlag.useMetallicRoughnessMap, 1.0);
}