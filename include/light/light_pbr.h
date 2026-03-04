#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include "shader/shader.h"

class Light {
public:
    glm::vec3 color;
    float intensity;

    Light(const glm::vec3& color = glm::vec3(1.0f), float intensity = 1.0f)
        : color(color), intensity(intensity) {}

    glm::vec3 getRadiance() const {
        return color * intensity;
    }
};

class DirLight : public Light {
public:
    glm::vec3 direction;

    DirLight(const glm::vec3& direction,
             const glm::vec3& color = glm::vec3(1.0f),
             float intensity = 1.0f)
        : Light(color, intensity), direction(glm::normalize(direction)) {}

    void setUniform(Shader &shader) {
        shader.setVec3("dirLight.direction", direction);
        shader.setVec3("dirLight.color", getRadiance());
    }
};

class PointLight : public Light {
public:
    glm::vec3 position;
    float radius;

    PointLight(const glm::vec3& position,
               const glm::vec3& color = glm::vec3(1.0f),
               float intensity = 1.0f,
               float radius = 1.0f)
        : Light(color, intensity), position(position), radius(radius) {}

    void setUniform(Shader &shader) {
        shader.setVec3("pointLight.position", position);
        shader.setVec3("pointLight.color", getRadiance());
        shader.setFloat("pointLight.radius", radius);
    }
};

class SpotLight : public Light {
public:
    glm::vec3 position;
    glm::vec3 direction;
    float cutOff;
    float outerCutOff;

    SpotLight(const glm::vec3& position,
              const glm::vec3& direction,
              const glm::vec3& color = glm::vec3(1.0f),
              float intensity = 1.0f,
              float cutOff = glm::cos(glm::radians(12.5f)),
              float outerCutOff = glm::cos(glm::radians(17.5f)))
        : Light(color, intensity), position(position), direction(glm::normalize(direction)), cutOff(cutOff), outerCutOff(outerCutOff) {}

    void setUniform(Shader &shader) {
        shader.setVec3("spotLight.color", getRadiance());
        shader.setVec3("spotLight.position", position);
        shader.setVec3("spotLight.direction", glm::normalize(direction));
        shader.setFloat("spotLight.cutOff", cutOff);
        shader.setFloat("spotLight.outerCutOff", outerCutOff);
    }

    void shut(Shader &shader) {
        shader.setVec3("spotLight.color", glm::vec3(0.0f));
    }
};

#endif