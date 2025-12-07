#include "Camera.h"

void Camera::UpdateViewMatrix() {
    // Calculate the camera basis vectors
    glm::vec3 forward = glm::normalize(target_ - position_);
    glm::vec3 right = glm::normalize(glm::cross(forward, up_));
    glm::vec3 up = glm::cross(right, forward);

    
    view_matrix_ = glm::mat4(1.0f);

    view_matrix_[0][0] = right.x;
    view_matrix_[1][0] = right.y;
    view_matrix_[2][0] = right.z;

    view_matrix_[0][1] = up.x;
    view_matrix_[1][1] = up.y;
    view_matrix_[2][1] = up.z;

    view_matrix_[0][2] = -forward.x;
    view_matrix_[1][2] = -forward.y;
    view_matrix_[2][2] = -forward.z;

    view_matrix_[3][0] = -glm::dot(right, position_);
    view_matrix_[3][1] = -glm::dot(up, position_);
    view_matrix_[3][2] = glm::dot(forward, position_);
}

void Camera::UpdateProjectionMatrix() {
    float aspect_ratio = static_cast<float>(width_) / static_cast<float>(height_);
    projection_matrix = glm::perspective(
        glm::radians(static_cast<float>(fovy_)),
        aspect_ratio,
        nearplane_,
        farplane_
    );
}

// View matrix getters
const glm::vec3 Camera::GetPosition() {
    return position_;
}

const glm::vec3 Camera::GetTarget() {
    return target_;
}

const glm::vec3 Camera::GetUp() {
    return up_;
}

const glm::mat4 Camera::GetViewMatrix() const {
    return view_matrix_;
}

const glm::mat4 Camera::GetProjectionMatrix() const {
    return projection_matrix;
}

// Projection matrix getters
const float Camera::GetFar() {
    return farplane_;
}

const float Camera::GetNear() {
    return nearplane_;
}

const double Camera::GetHeight() {
    return height_;
}

const double Camera::GetWidth() {
    return width_;
}

const double Camera::GetFOV() {
    return fovy_;
}

// Setters
void Camera::SetPosition(glm::vec3 position) {
    position_ = position;
    UpdateViewMatrix();
}

void Camera::SetTarget(glm::vec3 target) {
    target_ = target;
    UpdateViewMatrix();
}

void Camera::SetUp(glm::vec3 up) {
    up_ = up;
    UpdateViewMatrix();
}

void Camera::SetFOV(double fov) {
    fovy_ = fov;
    UpdateProjectionMatrix();
}