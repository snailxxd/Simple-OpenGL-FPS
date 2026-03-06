#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "animation.h"

class Animator {
public:
    Animator(Animation* Animation) {
        m_CurrentTime = 0.0;
        m_CurrentAnimation = Animation;
        m_speed = 1.0f;
        m_Looping = true;
        m_Finished = false;

        m_BoneCaches.assign(m_CurrentAnimation->GetBoneInfoMap().size(), {0, 0, 0});

        m_FinalBoneMatrices.reserve(100);
        for (int i = 0; i < 100; i++)
            m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
    }

    float& GetSpeed() { return m_speed; }

    Animation* GetCurrentAnimation() { return m_CurrentAnimation; }

    bool IsFinished() { return m_Finished; }

    void UpdateAnimation(float dt) {
        if (m_Finished) return;
        m_DeltaTime = dt;
        float duration = m_CurrentAnimation->GetDuration();
        if (m_CurrentAnimation) {
            m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt * m_speed;
            if (m_CurrentTime >= duration) {
                if (m_Looping) {
                    m_CurrentTime = fmod(m_CurrentTime, duration);
                    m_BoneCaches.assign(m_CurrentAnimation->GetBoneInfoMap().size(), {0, 0, 0});
                } else {
                    m_CurrentTime = duration - 1; 
                    m_Finished = true;
                }
            }
            CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
        }
    }

    void PlayAnimation(Animation* pAnimation, bool loop = true) {
        m_CurrentAnimation = pAnimation;
        m_CurrentTime = 0.0f;
        m_Looping = loop;
        m_BoneCaches.assign(pAnimation->GetBoneInfoMap().size(), {0, 0, 0});
        m_Finished = false;
    }

    void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform) {
        std::string nodeName = node->name;
        glm::mat4 nodeTransform = node->transformation;

        Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

        if (Bone) {
            int boneID = Bone->GetBoneID();
            BoneCache& cache = m_BoneCaches[boneID];
            Bone->Update(m_CurrentTime, cache);
            nodeTransform = Bone->GetLocalTransform();
        }

        glm::mat4 globalTransformation = parentTransform * nodeTransform;

        auto boneInfoMap = m_CurrentAnimation->GetBoneInfoMap();
        if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
            int index = boneInfoMap[nodeName].id;
            glm::mat4 offset = boneInfoMap[nodeName].offset;
            m_FinalBoneMatrices[index] = globalTransformation * offset;
        }

        for (int i = 0; i < node->childrenCount; i++)
            CalculateBoneTransform(&node->children[i], globalTransformation);
    }

    std::vector<glm::mat4> GetFinalBoneMatrices() { 
        return m_FinalBoneMatrices;  
    }

private:
    std::vector<glm::mat4> m_FinalBoneMatrices;
    Animation* m_CurrentAnimation;
    double m_CurrentTime;
    float m_DeltaTime;
    std::vector<BoneCache> m_BoneCaches;
    float m_speed;
    bool m_Looping;
    bool m_Finished;
};