#version 450
// input vertex data
layout(location = 0) in vec3 inPosition;  // Location 0
layout(location = 1) in vec3 inNormal;    // Location 1
layout(location = 2) in vec2 inTexCoords; // Location 2
layout(std140,set = 0, binding = 0) uniform CameraParam
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec3 cameraPosition; // Camera position in world space
}
cameraParam;

layout(push_constant) uniform PushConstant
{
    mat4 modelMatrix;
}
pushConstant;

// output fragments data
layout(location = 0) out vec3 outPosition; // Position in clip space
layout(location = 1) out vec3 outNormal; 
layout(location = 2) out vec2 outTexCoords;


void main()
{
    gl_Position = cameraParam.projectionMatrix * cameraParam.viewMatrix * pushConstant.modelMatrix * vec4(inPosition, 1.0);
    outNormal = normalize(mat3(pushConstant.modelMatrix) * inNormal); // Transform normal to world space
    outTexCoords = inTexCoords;
    outPosition = (pushConstant.modelMatrix * vec4(inPosition, 1.0)).rgb; // Position in world space
}


