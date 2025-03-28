#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec4 color;
layout(location = 2) in vec2 texCoords;
layout(set = 0, binding = 1) uniform sampler2D albedoTexture;
void main()
{
    vec4 texColor = texture(albedoTexture, texCoords);
    outColor = texColor;
}