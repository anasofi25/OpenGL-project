#include "Camera.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace gps {

   

    // Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        // Calculate initial camera front direction
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);

        // Right vector calculation
        this->cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));

        // Set initial yaw and pitch (could be passed as parameters too)
        yaw = -90.0f;  // Default value, looking straight ahead
        pitch = 0.0f;  // Default value, level with the ground
        updateCameraVectors();
    }

    void Camera::ProcessMouseMovement(float xOffset, float yOffset) {
        const float sensitivity = 0.1f; // Adjust mouse sensitivity as needed
        xOffset *= sensitivity;
        yOffset *= sensitivity;

        yaw += xOffset;
        pitch += yOffset;

        // Constrain pitch to prevent the camera from flipping over
        if (pitch > 89.0f) {
            pitch = 89.0f;
        }
        if (pitch < -89.0f) {
            pitch = -89.0f;
        }

        // Update the camera's front vector based on the new angles
        updateCameraVectors();
    }

    void Camera::updateCameraVectors() {
        // Calculate the new front vector based on yaw and pitch
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFrontDirection = glm::normalize(front);

        // Recalculate the right and up vectors
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
        cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
    }

    // Return the view matrix using glm::lookAt()
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, cameraUpDirection);
    }

    // Update the camera position based on the movement direction
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        if (direction == MOVE_FORWARD) {
            cameraPosition += speed * cameraFrontDirection;
        }
        if (direction == MOVE_BACKWARD) {
            cameraPosition -= speed * cameraFrontDirection;
        }
        if (direction == MOVE_LEFT) {
            cameraPosition -= speed * cameraRightDirection;
        }
        if (direction == MOVE_RIGHT) {
            cameraPosition += speed * cameraRightDirection;
        }
        if (direction == MOVE_UP)
            cameraPosition += cameraUpDirection * speed;  // Move along the up vector
        if (direction == MOVE_DOWN)
            cameraPosition -= cameraUpDirection * speed;
    }

    // Update the camera's front direction based on pitch and yaw angles
    void Camera::rotate(float pitch, float yaw) {
        // Convert pitch and yaw to radians
        float pitchRad = glm::radians(pitch);
        float yawRad = glm::radians(yaw);

        // Update camera front direction using spherical coordinates
        cameraFrontDirection.x = cos(pitchRad) * sin(yawRad);
        cameraFrontDirection.y = sin(pitchRad);
        cameraFrontDirection.z = cos(pitchRad) * cos(yawRad);

        cameraFrontDirection = glm::normalize(cameraFrontDirection);

        // Recalculate camera right and up directions
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
        cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
    }

    void Camera::setPosition(glm::vec3 position) {
        this->cameraPosition = position;
        updateCameraVectors();
    }

 
}
