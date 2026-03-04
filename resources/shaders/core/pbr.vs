#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in ivec4 aBoneIds;
layout (location = 6) in vec4 aBoneWeights;

out vec3 FragPos;
out vec2 TexCoords;
out vec4 FragPosLightSpace;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;
uniform mat4 lightSpaceMatrix;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

void main()
{
    mat4 boneTransform = mat4(0.0);
    float totalWeight = 0.0;
    for(int i = 0; i < MAX_BONE_INFLUENCE; i++) {
        if (aBoneIds[i] == -1) {
            break;
        }
        if (aBoneIds[i] >= MAX_BONES) {
            boneTransform = mat4(1.0);
            break;
        }
        boneTransform += aBoneWeights[i] * finalBonesMatrices[aBoneIds[i]];
        totalWeight += aBoneWeights[i];
    }
    if (totalWeight < 0.1) {
        boneTransform = mat4(1.0);
    }
    
    vec4 localPosition = boneTransform * vec4(aPos, 1.0);

    gl_Position = projection * view * model * localPosition;
    TexCoords = aTexCoords;
    FragPos = vec3(model * localPosition);
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
    
    mat3 skinMatrix = mat3(boneTransform);
    vec3 T = normalize(normalMatrix * skinMatrix * aTangent);
    vec3 N = normalize(normalMatrix * skinMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(T, N);

    TBN = mat3(T, B, N);
}