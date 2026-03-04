#include "animation/bone.h"

Bone::Bone(const std::string& name, int ID, const aiNodeAnim* channel)
: m_Name(name), m_ID(ID), m_LocalTransform(1.0f) {
    m_NumPositions = channel->mNumPositionKeys;
    m_NumRotations = channel->mNumRotationKeys;
    m_NumScales = channel->mNumScalingKeys;
    // 平移关键帧
    for (int positionIndex = 0; positionIndex < m_NumPositions; positionIndex++) {
        aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
        float timeStamp = channel->mPositionKeys[positionIndex].mTime;
        KeyPosition data;
        data.position = glm::vec3(aiPosition.x, aiPosition.y, aiPosition.z);
        data.timeStamp = timeStamp;
        m_Positions.push_back(data);
    }
    // 旋转关键帧
    for (int rotationIndex = 0; rotationIndex < m_NumRotations; rotationIndex++) {
        aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
        float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
        KeyRotation data;
        data.orientation = glm::quat(aiOrientation.w, aiOrientation.x, aiOrientation.y, aiOrientation.z);
        data.timeStamp = timeStamp;
        m_Rotations.push_back(data);
    }
    // 缩放关键帧
    for (int scaleIndex = 0; scaleIndex < m_NumScales; scaleIndex++) {
        aiVector3D aiScale = channel->mScalingKeys[scaleIndex].mValue;
        float timeStamp = channel->mScalingKeys[scaleIndex].mTime;
        KeyScale data;
        data.scale = glm::vec3(aiScale.x, aiScale.y, aiScale.z);
        data.timeStamp = timeStamp;
        m_Scales.push_back(data);
    }
}

void Bone::Update(float animationTime, BoneCache& cache) {
    int positionIndex = GetPositionIndex(animationTime, cache.lastPositionIndex);
    int rotationIndex = GetRotationIndex(animationTime, cache.lastRotationIndex);
    int scaleIndex = GetScaleIndex(animationTime, cache.lastScaleIndex);

    // 计算插值
    glm::mat4 translation = InterpolatePosition(animationTime, positionIndex);
    glm::mat4 rotation = InterpolateRotation(animationTime, rotationIndex);
    glm::mat4 scale = InterpolateScale(animationTime, scaleIndex);
    
    // 计算变换矩阵
    m_LocalTransform = translation * rotation * scale;

    // 更新缓存
    cache.lastPositionIndex = positionIndex;
    cache.lastRotationIndex = rotationIndex;
    cache.lastScaleIndex = scaleIndex;
}

float Bone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) {
    return (animationTime - lastTimeStamp) / (nextTimeStamp - lastTimeStamp);
}

glm::mat4 Bone::InterpolatePosition(float animationTime, int index) {
    if (m_NumPositions == 1) {
        return glm::translate(glm::mat4(1.0f), m_Positions[0].position);
    }
    int p0Index = index;
    int p1Index = index + 1;
    float scaleFactor = GetScaleFactor(m_Positions[p0Index].timeStamp, m_Positions[p1Index].timeStamp, animationTime);
    glm::vec3 finalPosition = glm::mix(m_Positions[p0Index].position, m_Positions[p1Index].position, scaleFactor);
    return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 Bone::InterpolateRotation(float animationTime, int index) {
    if (m_NumRotations == 1) {
        auto rotation = glm::normalize(m_Rotations[0].orientation);
        return glm::toMat4(rotation);
    }
    int r0Index = index;
    int r1Index = index + 1;
    float scaleFactor = GetScaleFactor(m_Rotations[r0Index].timeStamp, m_Rotations[r1Index].timeStamp, animationTime);
    glm::quat finalRotation = glm::slerp(m_Rotations[r0Index].orientation, m_Rotations[r1Index].orientation, scaleFactor);
    finalRotation = glm::normalize(finalRotation);
    return glm::toMat4(finalRotation);
}

glm::mat4 Bone::InterpolateScale(float animationTime, int index) {
    if (m_NumScales == 1) {
        return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);
    }
    int s0Index = index;
    int s1Index = index + 1;
    float scaleFactor = GetScaleFactor(m_Scales[s0Index].timeStamp, m_Scales[s1Index].timeStamp, animationTime);
    glm::vec3 finalScale = glm::mix(m_Scales[s0Index].scale, m_Scales[s1Index].scale, scaleFactor);
    return glm::scale(glm::mat4(1.0f), finalScale);
}

// 辅助函数，用于获取关键帧的索引
int Bone::GetPositionIndex(float animationTime, int lastIndex) {
    if (lastIndex < 0) {
        lastIndex = 0;
    }
    for (int index = lastIndex; index < m_NumPositions - 1; index++) {
        if (animationTime < m_Positions[index + 1].timeStamp) {
            return index;
        }
    }
    return 0;
}

int Bone::GetRotationIndex(float animationTime, int lastIndex) {
    if (lastIndex < 0) {
        lastIndex = 0;
    }
    for (int index = lastIndex; index < m_NumRotations - 1; index++) {
        if (animationTime < m_Rotations[index + 1].timeStamp) {
            return index;
        }
    }
    return 0;
}

int Bone::GetScaleIndex(float animationTime, int lastIndex) {
    if (lastIndex < 0) {
        lastIndex = 0;
    }
    for (int index = lastIndex; index < m_NumScales - 1; index++) {
        if (animationTime < m_Scales[index + 1].timeStamp) {
            return index;
        }
    }
    return 0;
}