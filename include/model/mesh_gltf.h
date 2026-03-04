#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader/shader.h"

#include <string>
#include <vector>

#define MAX_BONE_INFLUENCE 4

using namespace std;

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;

    glm::vec3 Tangent;
    glm::vec3 Bitangent;

    int m_BoneIDs[MAX_BONE_INFLUENCE];
    float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture {
    unsigned int id;
    string type;
    string path;
};

struct MatrialFactors {
    glm::vec4 baseColor = glm::vec4(1.0f);
    float metallic = 1.0f;
    float roughness = 1.0f;
};

class Mesh {
public:
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    MatrialFactors factors;

    glm::mat4 modelMatrix;  // mesh 的变换矩阵

    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures, glm::mat4 matrix = glm::mat4(1.0f), MatrialFactors factors = MatrialFactors()) {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
        this->modelMatrix = matrix;
        this->factors = factors;
        setupMesh();
    }

    void Draw(Shader &shader, glm::mat4 worldModel = glm::mat4(1.0f)) {
        // 计算 mesh 的变换矩阵
        glm::mat4 model = worldModel * modelMatrix;
        shader.setMat4("model", model);

        // 计算法线矩阵
        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
        shader.setMat3("normalMatrix", normalMatrix);

        // 设置材质参数
        shader.setVec4("material.baseColorFactor", factors.baseColor);
        shader.setFloat("material.metallicFactor", factors.metallic);
        shader.setFloat("material.roughnessFactor", factors.roughness);

        for(unsigned int i = 0; i < textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i); // 在绑定之前激活相应的纹理单元
            string name = textures[i].type;
            shader.setInt(("material." + name).c_str(), i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        // 绘制网格
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
    }
private:
    unsigned int VAO, VBO, EBO;

    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // 设置顶点属性指针
        // 顶点位置
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // 顶点法线
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // 顶点纹理坐标
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        // 切线
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        // 副切线
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
        // 骨骼索引
        glEnableVertexAttribArray(5);
        glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));
        // 骨骼权重
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));

        glBindVertexArray(0);
    }
};  

#endif
