#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include "shader/shader.h"

class Light {
public:
    glm::vec3 color;
    float ambientStrength;
    float diffuseStrength;
    float specularStrength;

    Light(const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        float ambientStrength = 0.1f,
        float diffuseStrength = 0.5f,
        float specularStrength = 1.0f)
    : color(color),
        ambientStrength(ambientStrength),
        diffuseStrength(diffuseStrength),
        specularStrength(specularStrength) {}
};

class DirLight : public Light {
public:
    glm::vec3 direction;

    DirLight(const glm::vec3& direction,
             const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
             float ambientStrength = 0.1f,
             float diffuseStrength = 0.5f,
             float specularStrength = 1.0f)
        : Light(color, ambientStrength, diffuseStrength, specularStrength),
          direction(glm::normalize(direction)) {}

    void setUniform(Shader &shader) {
        shader.setVec3("dirLight.direction", direction);
        shader.setVec3("dirLight.ambient", color * ambientStrength);   // 环境光照
        shader.setVec3("dirLight.diffuse", color * diffuseStrength);   // 漫反射光照
        shader.setVec3("dirLight.specular", color * specularStrength);  // 镜面光照
    }
};

class PointLight : public Light {
public:
    glm::vec3 position;
    float constant;
    float linear;
    float quadratic;

    PointLight(const glm::vec3& position,
               float constant = 1.0f,
               float linear = 0.09f,
               float quadratic = 0.032f,
               const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
               float ambientStrength = 0.1f,
               float diffuseStrength = 0.8f,
               float specularStrength = 1.0f)
    : Light(color, ambientStrength, diffuseStrength, specularStrength),
        position(position),
        constant(constant),
        linear(linear),
        quadratic(quadratic) {}

    void setUniform(Shader &shader) {
        shader.setVec3("pointLight.position", position);
        shader.setVec3("pointLight.ambient", color * ambientStrength);   // 环境光照
        shader.setVec3("pointLight.diffuse", color * diffuseStrength);   // 漫反射光照
        shader.setVec3("pointLight.specular", color * specularStrength);  // 镜面光照
        shader.setFloat("pointLight.constant", constant);
        shader.setFloat("pointLight.linear", linear);
        shader.setFloat("pointLight.quadratic", quadratic);
    }
};

class SpotLight : public Light {
public:
    glm::vec3 position;
    glm::vec3 direction;

    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    SpotLight(const glm::vec3& position,
              const glm::vec3& direction,
              float cutOff = glm::cos(glm::radians(12.5f)),
              float outerCutOff = glm::cos(glm::radians(17.5f)),
              float constant = 1.0f,
              float linear = 0.09f,
              float quadratic = 0.032f,
              const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
              float ambientStrength = 0.1f,
              float diffuseStrength = 0.8f,
              float specularStrength = 1.0f)
        : Light(color, ambientStrength, diffuseStrength, specularStrength),
          position(position),
          direction(glm::normalize(direction)),
          cutOff(cutOff),
          outerCutOff(outerCutOff),
          constant(constant),
          linear(linear),
          quadratic(quadratic) {}

    void setUniform(Shader &shader) {
        shader.setVec3("spotLight.position", position);
        shader.setVec3("spotLight.direction", direction);
        shader.setVec3("spotLight.ambient", color * ambientStrength);   // 环境光照
        shader.setVec3("spotLight.diffuse", color * diffuseStrength);   // 漫反射光照
        shader.setVec3("spotLight.specular", color * specularStrength); // 镜面光照
        shader.setFloat("spotLight.cutOff", cutOff);
        shader.setFloat("spotLight.outerCutOff", outerCutOff);
        shader.setFloat("spotLight.constant", constant);
        shader.setFloat("spotLight.linear", linear);
        shader.setFloat("spotLight.quadratic", quadratic);
    }

    void shut(Shader &shader) {
        shader.setVec3("spotLight.ambient", glm::vec3(0.0f, 0.0f, 0.0f));
        shader.setVec3("spotLight.diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
        shader.setVec3("spotLight.specular", glm::vec3(0.0f, 0.0f, 0.0f));
    }
};

#endif