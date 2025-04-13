#version 450

layout(location = 0) in vec3 inPosition;  // Location 0
layout(location = 1) in vec3 inNormal;    // Location 1
layout(location = 2) in vec2 inTexCoords; // Location 2
layout(location = 2) out vec2 outTexCoords;

layout(set = 0, binding = 0) uniform UniformBuffer
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
}
transform;

layout(push_constant) uniform PushConstant
{
    mat4 modelMatrix;
}
pushConstant;

void main()
{
    gl_Position = transform.projectionMatrix * transform.viewMatrix * pushConstant.modelMatrix * vec4(inPosition, 1.0);
    outTexCoords = inTexCoords;
}