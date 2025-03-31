#version 450

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;
layout(location = 3) out vec4 outMetallicRoughness;
layout(location = 4) out vec4 outAO;
void main()
{
    outPosition = vec4(gl_FragCoord.x, gl_FragCoord.y, 0.0, 1.0);
    outNormal = vec4(0.0, 0.0, 1.0, 1.0);
    outAlbedo = vec4(1.0, 0.0, 0.0, 1.0);
    outMetallicRoughness = vec4(0.0, 0.0, 0.0, 1.0);
    outAO = vec4(1.0, 1.0, 1.0, 1.0);
}