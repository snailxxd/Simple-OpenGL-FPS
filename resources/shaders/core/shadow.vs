#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 5) in ivec4 aBoneIds;
layout (location = 6) in vec4 aBoneWeights;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

void main() {
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

    gl_Position = lightSpaceMatrix * model * localPosition;
}