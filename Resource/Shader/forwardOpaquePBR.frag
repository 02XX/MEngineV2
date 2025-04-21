#version 450

const float PI = 3.14159265359f;

float DistributionGGX(vec3 N,vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NOH = clamp(dot(N,H),0,1.f);
	float NOH2 = NOH*NOH;
	float nom = a2;
	float demom = (NOH2 * (a2-1.0f) + 1.0f);
	demom = PI * demom * demom;
	return nom/demom;
}
float GeometrySchlickGGX(float NoV, float roughness)
{
	float r = roughness + 1.0f;
	float k = r*r / 8.0f;
	float nom = NoV;
	float denom = NoV * (1.0f - k) + k;
	return nom/denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NoV = clamp(dot(N,V),0,1.f);
	float NoL = clamp(dot(N,L),0,1.f);
	float ggx2 = GeometrySchlickGGX(NoV,roughness);
	float ggx1 = GeometrySchlickGGX(NoL,roughness);
	return ggx1* ggx2;
}
vec3 FresnelSchlick(float HoV, vec3 F0) {
	//钳制是否正确
    return F0 + (1.0 - F0) * pow(clamp(1.0-HoV,0.0,1.0), 5.0);
}
float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;// / 0x100000000
}
vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}
vec3 uniformSampleHemisphere(vec2 Xi, vec3 N) {
	float theta = acos(1.0 - Xi.x);
	float phi = 2.0f * PI * Xi.y;
	float x = sin(theta) * cos(phi);
	float y = sin(phi)*sin(theta);
	float z = cos(theta);
	vec3 up   = abs(N.y) < 0.999 ? vec3(0.0, 0.1, 0.0) : vec3(0.0, 0.0, 1.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
	mat3 TBN = mat3(tangent,bitangent,N);
	vec3 dir = TBN * vec3(x,y,z);
	return dir;
}
vec3 ImportanceSampleGGX(vec3 N, float roughness, vec2 Xi)
{
	  float a = roughness*roughness;
	float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	float x =sinTheta * cos(phi);
	float y = sin(phi)*sinTheta;
	float z = cosTheta;
	vec3 up   = abs(N.y) < 0.999 ? vec3(0.0, 0.1, 0.0) : vec3(0.0, 0.0, 1.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
	mat3 TBN = mat3(tangent,bitangent,N);
	vec3 dir = TBN * vec3(x,y,z);
	return dir;
}





const int MAX_LIGHT_COUNT = 6;
const int LightType_Directional = 0;
const int LightType_Point = 1;
const int LightType_Spot = 2;
const int LightType_Area = 3;
//input fragment data
layout(location = 0) in vec3 fragPosition; // Vertex position in world space
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
layout(std140,set = 0, binding = 0) uniform CameraParam
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec3 cameraPosition; // Camera position in world space

}
cameraParam;
layout(std140, set = 0, binding = 1) uniform LightParam{
        vec3 position; // 位置
        float range;        // 范围

        vec3 direction; // 方向
        float coneAngle;     // 锥角 only for spot light

        vec3 color; // 颜色
        float intensity; // 强度

        int lightType; // 光源类型 0:平行光 1:点光源 2:聚光灯 3:区域光源
} lights[MAX_LIGHT_COUNT];

layout(std140, set = 1, binding = 0) uniform MaterialParams
{
    PBRParameters parameters;
    PBRTextureFlag textureFlag;
}
pbrMaterial;



layout(set = 1, binding = 1) uniform sampler2D AlbedoMap;

layout(set = 0, binding = 2) uniform sampler2D EnvMap;
//output fragment data
//Render Targets
layout(location = 0) out vec4 outColor; // Color output
vec2 DirectionToUV(vec3 dir) {
    float theta = atan(dir.z, dir.x); // 水平角
    float phi = acos(dir.y);          // 垂直角
    float u = theta / (2.0 * 3.14159);
    float v = phi / 3.14159;
    return vec2(u, v); // UV 范围 [0, 1]
}
void main()
{
    outColor = vec4(0.0, 0.0, 0.0, 1.0);
    vec3 albedoColor = pbrMaterial.textureFlag.useAlbedoMap ? pbrMaterial.parameters.albedo * texture(AlbedoMap, fragTexCoord).rgb : pbrMaterial.parameters.albedo;
	float metallic = pbrMaterial.textureFlag.useMetallicRoughnessMap ? pbrMaterial.parameters.metallic * texture(AlbedoMap, fragTexCoord).b : pbrMaterial.parameters.metallic;
	float roughness = pbrMaterial.textureFlag.useMetallicRoughnessMap ? pbrMaterial.parameters.roughness * texture(AlbedoMap, fragTexCoord).g : pbrMaterial.parameters.roughness;
	// for(int i = 0; i < MAX_LIGHT_COUNT; i++)
    // {
        vec3 VIEW = cameraParam.cameraPosition - fragPosition;
        vec3 LIGHT = lights[0].direction;
        vec3 LIGHT_COLOR = lights[0].color * lights[0].intensity;
        vec3 NORMAL = normalize(fragNormal);
        vec3 V = normalize(VIEW);
        vec3 L = normalize(LIGHT);
        vec3 N = normalize(NORMAL);
        vec3 H = normalize(V+L);
        vec3 F0 = mix(vec3(0.04), albedoColor, metallic);
        float VoH = clamp(dot(V,H),0,1.0f);
        float NoV = clamp(dot(N,V),0,1.0f);
        float NoL = clamp(dot(N,L),0,1.0f);
        if(NoV > 0.0f)
        {
            vec3 F = FresnelSchlick(VoH, F0);
            float D = DistributionGGX(N,H,roughness);
            float G = GeometrySmith(N,V,L,roughness);
            vec3 numerator = F*D*G;
            float denominator = 4.0 * NoV * NoL + 1e-5;
            vec3 specular = numerator / denominator;
            vec3 specularColor = specular * LIGHT_COLOR * NoL;
            vec3 kS = F;
            vec3 kD = vec3(1.0) - kS;
            kD *= 1.0 - metallic;
            outColor += vec4((kD*albedoColor/PI + specular) * (LIGHT_COLOR) * NoL, 1.0);
        }
        //environment
        const uint SAMPLE_COUNT = 1024u;
        vec3 envSpecular = vec3(0);
        vec3 envDiffuse = vec3(0);
        for(uint i = 0u; i < SAMPLE_COUNT; i++)
        {
            vec2 Xi = Hammersley(i, SAMPLE_COUNT);
            
            vec3 H = ImportanceSampleGGX(N, roughness,Xi);
            vec3 L = normalize(2.0 * dot(V, H) * H - V);
            float NdotL = clamp(dot(N,L),0,1.0f);
            float NdotH =clamp(dot(N,H),0,1.0f);
            float VdotH =clamp(dot(V,H),0,1.0f);
            float NdotV = clamp(dot(N,V),0,0.1f);
            
            if(NdotL > 0.0)
            {
                float G =GeometrySmith(N,V,L,roughness);
                vec3 F = FresnelSchlick(VdotH, F0);
                float D = DistributionGGX(N,H,roughness);
                vec3 numerator = F*D*G;
                float denominator = 4.0 * NdotV * NdotL + 1e-5;
                vec3 specular = numerator / denominator;
                vec4 environmentColor = texture(EnvMap, DirectionToUV(L));
                float pdf = D * NdotH/(4.0f * VdotH + 1e-5);
                envSpecular += specular * environmentColor.rgb * NdotL / pdf;
                environmentColor = texture(EnvMap, DirectionToUV(H));
                envDiffuse += (albedoColor/PI) *  environmentColor.rgb * NdotL / pdf;
            }
        }
        envSpecular /= float(SAMPLE_COUNT);
        envDiffuse /= float(SAMPLE_COUNT);
        outColor += vec4(envSpecular + envDiffuse, 1.0) ;
    // }

    // outColor = vec4(envColor,1.0);
}


