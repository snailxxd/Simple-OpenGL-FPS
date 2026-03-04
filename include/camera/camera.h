#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement {
    FORWARD,
    HORIZON_FORWARD,
    BACKWARD,
    HORIZON_BACKWARD,
    LEFT,
    RIGHT,
    UP,
    WORLD_UP,
    DOWN,
    WORLD_DOWN
};

const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  3.5f;
const float SENSITIVITY =  0.1f;
const float FOV         =  45.0f;

class Camera {
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    glm::vec3 HorizontalFront;

    float Yaw;
    float Pitch;

    float Fov;

    float MovementSpeed;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), Fov(FOV)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(Position, Position + Front, Up);
    }

    glm::mat4 GetProjectionMatrix(float aspectRatio) const {
        return glm::perspective(glm::radians(Fov), aspectRatio, 0.1f, 100.0f);
    }

    void Move(Camera_Movement direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == HORIZON_FORWARD)
            Position += HorizontalFront * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == HORIZON_BACKWARD)
            Position -= HorizontalFront * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
        if (direction == UP)
            Position += Up * velocity;
        if (direction == WORLD_UP)
            Position += WorldUp * velocity;
        if (direction == DOWN)
            Position -= Up * velocity;
        if (direction == WORLD_DOWN)
            Position -= WorldUp * velocity;
    }
    
    void PitchAndYaw(float xoffset, float yoffset) {
        Yaw   += xoffset;
        Pitch += yoffset;

        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;

        updateCameraVectors();
    }

    void PitchUp(float angle) {
        Pitch += angle;
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
        updateCameraVectors();
    }

    void PitchDown(float angle) {
        Pitch -= angle;
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
        updateCameraVectors();
    }

    void YawLeft(float angle) {
        Yaw -= angle;
        updateCameraVectors();
    }

    void YawRight(float angle) {
        Yaw += angle;
        updateCameraVectors();
    }

private:
    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up    = glm::normalize(glm::cross(Right, Front));
        HorizontalFront = glm::normalize(glm::cross(WorldUp, Right));
    }
};
#endif