#ifndef MODEL_H
#define MODEL_H

#include "mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stb_image.h>
#include <filesystem>
#include <map>

class Model 
{
public:
    map<string, Texture> textures_loaded;    // 保存已加载纹理，避免重复加载
    vector<Mesh> meshes;
    string directory;
    bool gammaCorrection;

    Model(const string &path, bool gamma = false) : gammaCorrection(gamma)
    {
        loadModel(path);
    }

    void Draw(Shader &shader) {
        for(unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

private:
    void loadModel(const string &path) {
        cout << "Loading model: " << path << endl;
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        std::filesystem::path fsPath(path);
        directory = fsPath.parent_path().string();                  // 设置目录

        processNode(scene->mRootNode, scene);                       // 处理根节点
    }

    void processNode(aiNode *node, const aiScene *scene) {
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {       // 处理节点所有网格
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        for (unsigned int i = 0; i < node->mNumChildren; i++) {     // 递归处理所有子节点
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh *mesh, const aiScene *scene) {
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {     // 处理顶点
            Vertex vertex;
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

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {        // 处理索引
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        float shininess = 128.0f;
        if (mesh->mMaterialIndex >= 0) {                           // 处理材质
            aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
            // diffuse maps
            vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", scene);
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            // specular maps
            vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", scene);
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
            // normal maps
            vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal", scene);
            textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
            vector<Texture> normalMapsStandard = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal", scene);
            textures.insert(textures.end(), normalMapsStandard.begin(), normalMapsStandard.end());
            // shininess
            aiGetMaterialFloat(material, AI_MATKEY_SHININESS, &shininess);
        }

        return Mesh(vertices, indices, textures, shininess);
    }

    vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName, const aiScene *scene) {
        vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
            aiString str;
            mat->GetTexture(type, i, &str);
            std::string texPath = str.C_Str();

            auto it = textures_loaded.find(texPath);
            if (it != textures_loaded.end()) {
                textures.push_back(it->second);
            } else {
                Texture texture;
                bool gamma = (typeName == "texture_diffuse") ? this->gammaCorrection : false;

                cout << "   Loading " << typeName << ": " << texPath << endl;
                const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(str.C_Str());
                if (embeddedTexture) {
                    texture.id = TextureFromMemory(embeddedTexture, gamma);
                } else {
                    texture.id = TextureFromFile(str.C_Str(), this->directory, gamma);
                }

                texture.type = typeName;
                texture.path = texPath;
                textures.push_back(texture);
                textures_loaded[texPath] = texture; 
            }
        }
        return textures;
    }

    unsigned int TextureFromMemory(const aiTexture* cachedTexture, bool gamma) {
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

    unsigned int TextureFromFile(const char *path, const string &directory, bool gamma) {
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
};

#endif