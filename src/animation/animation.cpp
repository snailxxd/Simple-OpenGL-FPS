#include "animation/animation.h"

Animation::Animation(aiAnimation* animation, aiNode* rootNode, std::map<std::string, BoneInfo>& boneInfoMap, int& boneCount) {
    m_Duration = animation->mDuration;
    m_TicksPerSecond = animation->mTicksPerSecond;
    ReadHierarchyData(m_RootNode, rootNode);
    ReadMissingBones(animation, boneInfoMap, boneCount);
}

Bone* Animation::FindBone(const std::string& name) {
    for (auto& bone : m_Bones) {
        if (bone.GetBoneName() == name) {
            return &bone;
        }
    }
    return nullptr;
}

void Animation::ReadHierarchyData(AssimpNodeData& dest, const aiNode* src) {
    dest.name = src->mName.data;
    dest.transformation = Utils::aiMatrix4x4ToGlm(src->mTransformation);
    dest.childrenCount = src->mNumChildren;
    // 递归读取子节点
    for (int i = 0; i < src->mNumChildren; i++) {
        AssimpNodeData child;
        ReadHierarchyData(child, src->mChildren[i]);
        dest.children.push_back(child);
    }
}

void Animation::ReadMissingBones(const aiAnimation* animation, std::map<std::string, BoneInfo>& boneInfoMap, int& boneCount) {
    int size = animation->mNumChannels;     // 获取骨骼数量

    for (int i = 0; i < size; i++)
    {
        auto channel = animation->mChannels[i];
        std::string boneName = channel->mNodeName.data;

        if (boneInfoMap.find(boneName) == boneInfoMap.end())
        {
            boneInfoMap[boneName].id = boneCount;
            boneCount++;
        }
        m_Bones.push_back(Bone(channel->mNodeName.data, boneInfoMap[channel->mNodeName.data].id, channel));
    }

    m_BoneInfoMap = boneInfoMap;
}
