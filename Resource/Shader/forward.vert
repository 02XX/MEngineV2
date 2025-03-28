#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;
layout(location = 0) out vec4 color;
void main()
{
    gl_Position = vec4(inPosition, 1.0);
    color = vec4(normal.x * 0.5 + 0.5, normal.y * 0.5 + 0.5, normal.z * 0.5 + 0.5, 1);
}