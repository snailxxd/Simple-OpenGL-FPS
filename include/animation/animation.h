#pragma once

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include "utils.h"
#include "bone.h"
#include "animdata.h"

struct AssimpNodeData {
    glm::mat4 transformation;
    std::string name;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};

class Animation {
public:
    Animation() = default;
    Animation(aiAnimation* animation, aiNode* rootNode, std::map<std::string, BoneInfo>& boneInfoMap, int& boneCount);

    Bone* FindBone(const std::string& name);

    float GetTicksPerSecond() { return m_TicksPerSecond; }
    float GetDuration() { return m_Duration; }
    const AssimpNodeData& GetRootNode() { return m_RootNode; }
    const std::map<std::string, BoneInfo>& GetBoneInfoMap() { return m_BoneInfoMap; }

private:
    float m_Duration;
    int m_TicksPerSecond;
    std::vector<Bone> m_Bones;
    AssimpNodeData m_RootNode;
    std::map<std::string, BoneInfo> m_BoneInfoMap;

    void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src);

    void ReadMissingBones(const aiAnimation* animation, std::map<std::string, BoneInfo>& boneInfoMap, int& boneCount);
};
