#version 450

layout(location = 0) in vec3 inPosition;  // Location 0
layout(location = 1) in vec3 inNormal;    // Location 1
layout(location = 2) in vec2 inTexCoords; // Location 2
void main()
{
    gl_Position = vec4(inPosition, 1.0);
}