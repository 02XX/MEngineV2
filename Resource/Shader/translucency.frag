#version 450

layout(location = 0) out vec4 outColor;
layout(location = 2) in vec2 inTexCoords;

layout(set = 1, binding = 0) uniform sampler2D baseColorSampler;
void main()
{
    outColor = texture(baseColorSampler, inTexCoords);
    // outColor = vec4(inTexCoords, 1.0, 1.0);
}