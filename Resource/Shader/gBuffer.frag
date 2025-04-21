#version 450

layout(location = 0) in vec3 inWorldPos; 
layout(location = 1) in vec3 inNormal;  
layout(location = 2) in vec2 inTexCoords;


layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outPosition;
layout(location = 3) out vec4 outMetallicRoughness;
layout(location = 4) out vec4 outAO;
layout(location = 5) out vec4 outEmissive;


layout(set=1, binding=1) uniform sampler2D albedoMap;
layout(set=1, binding=2) uniform sampler2D normalMap;
layout(set=1, binding=3) uniform sampler2D metallicRoughnessMap;
layout(set=1, binding=4) uniform sampler2D aoMap;
layout(set=1, binding=5) uniform sampler2D emissiveMap;

void main()
{
    outAlbedo = texture(albedoMap, inTexCoords);
    outNormal = vec4(normalize(inNormal), 1.0);
    outPosition = vec4(inWorldPos, 1.0);
    outMetallicRoughness = texture(metallicRoughnessMap, inTexCoords);
    outAO = texture(aoMap, inTexCoords);
    outEmissive = texture(emissiveMap, inTexCoords);
}