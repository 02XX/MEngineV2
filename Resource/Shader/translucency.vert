#version 450

layout(location = 0) in vec3 inPosition;  // Location 0
layout(location = 1) in vec3 inNormal;    // Location 1
layout(location = 2) in vec2 inTexCoords; // Location 2

layout(location = 0) out vec4 color;

layout(set = 0, binding = 0) uniform UniformBuffer
{
    mat4 modelMatrix;
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
    color = vec4(inNormal.x * 0.5 + 0.5, inNormal.y * 0.5 + 0.5, inNormal.z * 0.5 + 0.5, 1);
}