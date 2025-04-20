#version 450

//input fragment data
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
struct PBRParameters
{
    vec3 albedo; //sizeof(vec3) = 12 alignof(vec3) = 16
    float metallic;
    float roughness;
    float ao;
    float emissive;
};

struct PBRTextureFlag
{
    bool useAlbedoMap; //sizeof(bool) = 4 alignof(bool) = 4
    bool useNormalMap;
    bool useMetallicRoughnessMap;
    bool useAOMap;
    bool useEmissiveMap;
};
layout(std140, set = 1, binding = 0) uniform MaterialParams
{
    PBRParameters parameters;
    PBRTextureFlag textureFlag;
}
pbrMaterial;
layout(set = 1, binding = 1) uniform sampler2D AlbedoMap;

//output fragment data
//Render Targets
layout(location = 0) out vec4 outColor; // Color output

void main()
{
    if (pbrMaterial.textureFlag.useAlbedoMap)
    {
        outColor = texture(AlbedoMap, fragTexCoord);
    }
    else
    {
        outColor = vec4(pbrMaterial.parameters.albedo, 1.0);
    }
}


