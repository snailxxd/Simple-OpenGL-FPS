#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in vec4 FragPosLightSpace;
in mat3 TBN;

struct Material {
    sampler2D albedoMap;
    vec4 baseColorFactor;
    sampler2D normalMap;
    sampler2D ormMap;
    float roughnessFactor;
    float metallicFactor;
};

struct DirLight {
    vec3 direction;
    vec3 color;
    sampler2D shadowMap;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float radius;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float cutOff;
    float outerCutOff;
};

uniform Material material;
uniform DirLight dirLight;
// uniform PointLight pointLight;
uniform SpotLight spotLight;

uniform vec3 viewPos;

const float PI = 3.14159265359;

vec3 getNormalFromMap() {
    vec3 normal = texture(material.normalMap, TexCoords).rgb * 2.0 - 1.0;
    return normalize(TBN * normal);
}

float CalcShadow(vec4 fragPosLightSpace, sampler2D shadowMap, vec3 normal, vec3 lightDir) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0)
        return 0.0;

    float currentDepth = projCoords.z;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    return shadow;
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 CalcLight(vec3 L, vec3 V, vec3 N, vec3 radiance, vec3 F0, vec3 albedo, float roughness, float metallic) {
    vec3 H = normalize(L + V);

    // Cook-Torrance BRDF
    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 specular = (D * G * F) / (4.0 * max(dot(N, L), 0.0) * max(dot(N, V), 0.0) + 0.0001);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;     // 金属只有镜面反射

    return (kD * albedo / PI + specular) * radiance *  max(dot(N, L), 0.0);
}

void main()
{
    vec3 albedo = texture(material.albedoMap, TexCoords).rgb * material.baseColorFactor.rgb;
    float ao = texture(material.ormMap, TexCoords).r;
    float roughness = texture(material.ormMap, TexCoords).g * material.roughnessFactor;
    float metallic = texture(material.ormMap, TexCoords).b * material.metallicFactor;

    vec3 N = getNormalFromMap();            // 法线
    vec3 V = normalize(viewPos - FragPos);  // 视向量

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);         // 根据金属度自动计算反射因子

    vec3 Lo = vec3(0.0);

    // ----- 计算平行光 ----- 
    vec3 L_dir = normalize(-dirLight.direction);
    float shadow = 1.0 - CalcShadow(FragPosLightSpace, dirLight.shadowMap, N, L_dir);
    vec3 rad_dir = dirLight.color * shadow;
    Lo += CalcLight(L_dir, V, N, rad_dir, F0, albedo, roughness, metallic);

    // ----- 计算聚光灯 ----- 
    vec3 L_spot = normalize(-spotLight.direction);
    // 光照范围
    float theta = dot(normalize(spotLight.position - FragPos), L_spot);
    float epsilon = spotLight.cutOff - spotLight.outerCutOff;
    float intensity = clamp((theta - spotLight.outerCutOff) / epsilon, 0.0, 1.0);
    // 衰减
    float distance = length(spotLight.position - FragPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 rad_spot = spotLight.color * attenuation * intensity;
    Lo += CalcLight(L_spot, V, N, rad_spot, F0, albedo, roughness, metallic);

    // ----- 计算环境光 ----- 
    vec3 ambient = vec3(0.05) * albedo * ao;
    vec3 color = ambient + Lo;

    // HDR 色彩校正
    color = color / (color + vec3(1.0));

    FragColor = vec4(color, 1.0);
}