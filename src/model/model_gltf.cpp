#include "model/model_gltf.h"

Model::Model(const string &path, bool gamma) : gammaCorrection(gamma)
{
    m_BoneCounter = 0;
    initDefaultTextures();
    loadModel(path);
}

void Model::Draw(Shader &shader, glm::mat4 worldModel) {
    for(unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw(shader, worldModel);
}

void Model::SetVertexBoneDataToDefault(Vertex& vertex) {
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
        vertex.m_BoneIDs[i] = -1;
        vertex.m_Weights[i] = 0.0f;
    }
}

void Model::SetVertexBoneData(Vertex& vertex, int boneID, float weight) {
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
        if (vertex.m_BoneIDs[i] < 0) {
            vertex.m_BoneIDs[i] = boneID;
            vertex.m_Weights[i] = weight;
            break;
        }
    }
}

void Model::ExtractBoneWeightForVertices(vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene) {
    for (int boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++) {
        string boneName = mesh->mBones[boneIndex]->mName.C_Str();
        if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end()) {      // 检查骨骼是否已存在
            BoneInfo newBoneInfo;
            newBoneInfo.id = m_BoneCounter;
            newBoneInfo.offset = Utils::aiMatrix4x4ToGlm(mesh->mBones[boneIndex]->mOffsetMatrix);
            m_BoneInfoMap[boneName] = newBoneInfo;
            m_BoneCounter++;
        }
        int boneID = m_BoneInfoMap[boneName].id;

        int numWeights = mesh->mBones[boneIndex]->mNumWeights;
        auto weights = mesh->mBones[boneIndex]->mWeights;
        for (int weightIndex = 0; weightIndex < numWeights; weightIndex++) {
            int vertexID = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;
            SetVertexBoneData(vertices[vertexID], boneID, weight);
        }
    }
}

unsigned int Model::createColorTexture(unsigned char r, unsigned char g, unsigned char b, unsigned char a, bool gamma) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    unsigned char data[] = { r, g, b, a };
    glBindTexture(GL_TEXTURE_2D, textureID);

    GLenum internal_format = gamma? GL_SRGB_ALPHA : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return textureID;
}

void Model::initDefaultTextures() {
    defaultAlbedo = createColorTexture(255, 255, 255, 255, gammaCorrection);
    defaultNormal = createColorTexture(128, 128, 255, 255, false);
    defaultORM = createColorTexture(255, 255, 255, 255, false);
}

void Model::loadModel(const string &path) {
    cout << "Loading model: " << path << endl;
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, 
        aiProcess_Triangulate | 
        aiProcess_CalcTangentSpace |
        aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
        return;
    }
    std::filesystem::path fsPath(path);
    directory = fsPath.parent_path().string();                  // 设置目录

    processNode(scene->mRootNode, scene, glm::mat4(1.0f));      // 处理根节点

    // 提取动画
    int animationCount = scene->mNumAnimations;
    for (int i = 0; i < animationCount; i++) {
        string name = scene->mAnimations[i]->mName.C_Str();
        Animation animation(scene->mAnimations[i], scene->mRootNode, m_BoneInfoMap, m_BoneCounter);
        animations.emplace(name, animation);
        cout << "Animation loaded: " << name << ": " 
                << animation.GetDuration() << " ticks, " 
                << animation.GetTicksPerSecond() << " tps" << endl; 
    }
}

void Model::processNode(aiNode *node, const aiScene *scene, glm::mat4 parentTransform) {
    glm::mat4 nodeTransform = Utils::aiMatrix4x4ToGlm(node->mTransformation);
    glm::mat4 worldTransform = parentTransform * nodeTransform;

    for (unsigned int i = 0; i < node->mNumMeshes; i++) {       // 处理节点所有网格
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene, worldTransform));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) {     // 递归处理所有子节点
        processNode(node->mChildren[i], scene, worldTransform);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene, glm::mat4 transform) {
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    MatrialFactors factors;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {     // 顶点属性解析
        Vertex vertex;

        SetVertexBoneDataToDefault(vertex);                     // 初始化骨骼数据

        vertex.Position = glm::vec3(mesh->mVertices[i].x,       // 位置
                                    mesh->mVertices[i].y, 
                                    mesh->mVertices[i].z);

        vertex.Normal = glm::vec3(mesh->mNormals[i].x,          // 法线
                                    mesh->mNormals[i].y,
                                    mesh->mNormals[i].z);

        if (mesh->mTextureCoords[0]) {                          // 纹理坐标
            vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, 
                                            mesh->mTextureCoords[0][i].y);
            
            vertex.Tangent = glm::vec3(mesh->mTangents[i].x,
                                        mesh->mTangents[i].y,
                                        mesh->mTangents[i].z);

            vertex.Bitangent = glm::vec3(mesh->mBitangents[i].x,
                                            mesh->mBitangents[i].y,
                                            mesh->mBitangents[i].z);
        } else {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }
        vertices.push_back(vertex);
    }

    ExtractBoneWeightForVertices(vertices, mesh, scene);        // 提取骨骼权重

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {        // 处理索引
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // PBR 材质解析
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        // Base Color Factor
        aiColor4D baseColor(1.0f, 1.0f, 1.0f, 1.0f);
        if (AI_SUCCESS == material->Get(AI_MATKEY_BASE_COLOR, baseColor)) {
            factors.baseColor = glm::vec4(baseColor.r, baseColor.g, baseColor.b, baseColor.a);
        }

        // Metallic Factor
        float metallic = 1.0f;
        if (AI_SUCCESS == material->Get(AI_MATKEY_METALLIC_FACTOR, metallic)) {
            factors.metallic = metallic;
        }

        // Roughness Factor
        float roughness = 1.0f;
        if (AI_SUCCESS == material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness)) {
            factors.roughness = roughness;
        }

        // Albedo map
        textures.push_back(loadMaterialTexture(material, aiTextureType_BASE_COLOR, "albedoMap", scene));

        // Normal map
        textures.push_back(loadMaterialTexture(material, aiTextureType_NORMALS, "normalMap", scene));

        // ORMmap
        textures.push_back(loadMaterialTexture(material, aiTextureType_METALNESS, "ormMap", scene));
    }

    return Mesh(vertices, indices, textures, transform, factors);
}

Texture Model::loadMaterialTexture(aiMaterial *mat, aiTextureType type, string typeName, const aiScene *scene) {
    if (mat->GetTextureCount(type) > 0) {
        aiString str;
        mat->GetTexture(type, 0, &str);
        std::string texPath = str.C_Str();

        if (textures_loaded.find(texPath) != textures_loaded.end()) {
            return textures_loaded[texPath];
        } 

        Texture texture;
        bool gamma = (typeName == "albedoMap") ? this->gammaCorrection : false;

        cout << "   Loading " << typeName << ": " << texPath << endl;
        const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(str.C_Str());
        if (embeddedTexture) {
            texture.id = TextureFromMemory(embeddedTexture, gamma);
        } else {
            texture.id = TextureFromFile(str.C_Str(), this->directory, gamma);
        }

        texture.type = typeName;
        texture.path = texPath;
        textures_loaded[texPath] = texture; 
        return texture;
    }

    Texture defaultTexture;
    defaultTexture.type = typeName;
    defaultTexture.path = "default_" + typeName;

    if (typeName == "albedoMap") defaultTexture.id = defaultAlbedo;
    else if (typeName == "normalMap") defaultTexture.id = defaultNormal;
    else if (typeName == "ormMap") defaultTexture.id = defaultORM;

    return defaultTexture;
}

unsigned int Model::TextureFromMemory(const aiTexture* cachedTexture, bool gamma) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data;
    if (cachedTexture->mHeight == 0) {
        data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(cachedTexture->pcData), 
                                    cachedTexture->mWidth, &width, &height, &nrComponents, 0);
    } else {
        data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(cachedTexture->pcData), 
                                    cachedTexture->mWidth * cachedTexture->mHeight, &width, &height, &nrComponents, 0);
    }

    if (data) {
        GLenum format;
        GLenum internal_format;
        if (nrComponents == 1) {
            format = GL_RED;
            internal_format = GL_RED;
        } else if (nrComponents == 3) {
            format = GL_RGB;
            internal_format = gamma? GL_SRGB : GL_RGB;
        } else if (nrComponents == 4) {
            format = GL_RGBA;
            internal_format = gamma? GL_SRGB_ALPHA : GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (nrComponents == 1) {
            GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
            glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
        }

        stbi_image_free(data);
    } else {
        cout << "Texture failed to load from memory" << endl;
        stbi_image_free(data);
    }

    return textureID;
}

unsigned int Model::TextureFromFile(const char *path, const string &directory, bool gamma) {
    string filename = string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        GLenum internal_format;
        if (nrComponents == 1) {
            format = GL_RED;
            internal_format = GL_RED;
        } else if (nrComponents == 3) {
            format = GL_RGB;
            internal_format = gamma? GL_SRGB : GL_RGB;
        } else if (nrComponents == 4) {
            format = GL_RGBA;
            internal_format = gamma? GL_SRGB_ALPHA : GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (nrComponents == 1) {
            GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
            glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
        }

        stbi_image_free(data);
    } else {
        cout << "Texture failed to load at path: " << filename << endl;
        stbi_image_free(data);
    }

    return textureID;
}