#pragma once

#include <vector>
#include <string>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct BoneCache {
    int lastPositionIndex;
    int lastRotationIndex;
    int lastScaleIndex;
};

struct KeyPosition {
    glm::vec3 position;
    float timeStamp;
};

struct KeyRotation {
    glm::quat orientation;
    float timeStamp;
};

struct KeyScale {
    glm::vec3 scale;
    float timeStamp;
};

class Bone {
public:
    Bone(const std::string& name, int ID, const aiNodeAnim* channel);

    void Update(float animationTime, BoneCache& cache);

    glm::mat4 GetLocalTransform() const {
        return m_LocalTransform;
    }

    std::string GetBoneName() const {
        return m_Name;
    }

    int GetBoneID() const {
        return m_ID;
    }

private:
    std::vector<KeyPosition> m_Positions;
    std::vector<KeyRotation> m_Rotations;
    std::vector<KeyScale> m_Scales;
    int m_NumPositions;
    int m_NumRotations;
    int m_NumScales;

    glm::mat4 m_LocalTransform;
    std::string m_Name;
    int m_ID;

    float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);

    glm::mat4 InterpolatePosition(float animationTime, int index);
    glm::mat4 InterpolateRotation(float animationTime, int index);
    glm::mat4 InterpolateScale(float animationTime, int index);

    int GetPositionIndex(float animationTime, int lastIndex);
    int GetRotationIndex(float animationTime, int lastIndex);
    int GetScaleIndex(float animationTime, int lastIndex);
};