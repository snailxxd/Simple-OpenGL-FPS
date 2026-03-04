#ifndef MODEL_H
#define MODEL_H

#include "utils.h"
#include "mesh_gltf.h"
#include "animation/animdata.h"
#include "animation/animation.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stb_image.h>
#include <filesystem>
#include <map>
#include <algorithm>

class Model 
{
public:
    map<string, Texture> textures_loaded;   // 保存已加载纹理，避免重复加载
    vector<Mesh> meshes;                    // 存储所有网格
    string directory;                       // 模型所在目录
    bool gammaCorrection;                   // 是否开启伽马校正
    unsigned int defaultAlbedo, defaultNormal, defaultORM;  // 默认纹理
    map<string, Animation> animations;      // 存储模型的所有动画

    Model(const string &path, bool gamma = true);

    void Draw(Shader &shader, glm::mat4 worldModel = glm::mat4(1.0f));

    auto& GetBoneInfoMap() { return m_BoneInfoMap; }
    int& GetBoneCounter() { return m_BoneCounter; }

private:
    map<string, BoneInfo> m_BoneInfoMap;    // 存储骨骼信息
    int m_BoneCounter;                      // 骨骼数量

    void SetVertexBoneDataToDefault(Vertex& vertex);
    void SetVertexBoneData(Vertex& vertex, int boneID, float weight);
    void ExtractBoneWeightForVertices(vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);

    unsigned int createColorTexture(unsigned char r, unsigned char g, unsigned char b, unsigned char a, bool gamma);
    void initDefaultTextures();

    void loadModel(const string &path);

    void processNode(aiNode *node, const aiScene *scene, glm::mat4 parentTransform);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene, glm::mat4 transform);

    Texture loadMaterialTexture(aiMaterial *mat, aiTextureType type, string typeName, const aiScene *scene);

    unsigned int TextureFromMemory(const aiTexture* cachedTexture, bool gamma);
    unsigned int TextureFromFile(const char *path, const string &directory, bool gamma);
};

#endif