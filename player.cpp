#include "Player.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

Player::Player(Camera* camera) : camera_(camera) {
}

Player::~Player() {
    if (controller_) {
        controller_->release();
    }
}

void Player::Initialize(const CharacterControllerConfig& config) {
    position_ = config.position;
    velocity_ = glm::vec3(0.0f);

    // Create capsule controller with full configuration
    controller_ = PhysicsManager::Instance().CreateCapsuleController(config);

    UpdateCamera();
}

void Player::Initialize(const glm::vec3& spawn_position) {
    position_ = spawn_position;
    velocity_ = glm::vec3(0.0f);

    // Create character controller configuration
    CharacterControllerConfig config;
    config.position = spawn_position;
    config.radius = 0.8f;
    config.height = 1.8f;
    config.step_offset = 0.3f;
    config.slope_limit = 45.0f;
    config.contact_offset = 0.08f;
    config.max_jump_height = 2.0f;

    // Create controller
    controller_ = PhysicsManager::Instance().CreateCapsuleController(config);

    if (!controller_) {
        std::cerr << "Failed to create player controller!" << std::endl;
        // Continue without physics for now
    }
    else {
        std::cout << "Player controller created successfully!" << std::endl;
    }

    UpdateCamera();
}

void Player::SetPosition(const glm::vec3& pos) {
    position_ = pos;
    if (controller_) {
        controller_->setPosition(PxExtendedVec3(pos.x, pos.y, pos.z));
    }
    UpdateCamera();
}

void Player::Update(float delta_time) {
    // Always update camera for mouse look, even without physics controller
    UpdateCamera();

    if (!controller_) {
        // No physics controller - just do simple movement without collision
        HandleMovement(delta_time);
        position_ += velocity_ * delta_time;
        return;
    }

    // Handle movement
    HandleMovement(delta_time);

    // Apply gravity
    if (!is_grounded_) {
        velocity_.z -= 9.81f * delta_time;
    }
    else {
        if (velocity_.z < 0) velocity_.z = 0;
    }

    // Jump
    if (keys_pressed_[GLFW_KEY_SPACE] && is_grounded_) {
        velocity_.z = jump_force_;
    }

    // Move controller
    PxVec3 displacement = ToPxVec3(velocity_ * delta_time);
    PxControllerFilters filters;
    PxControllerCollisionFlags collision_flags = controller_->move(displacement, 0.001f, delta_time, filters);

    // Update position
    PxExtendedVec3 pos = controller_->getPosition();
    position_ = glm::vec3(pos.x, pos.y, pos.z);

    // Check grounded and collision flags
    is_grounded_ = collision_flags & PxControllerCollisionFlag::eCOLLISION_DOWN;

    // Optional: Check if hit sides (for sliding along walls)
    bool hit_sides = collision_flags & PxControllerCollisionFlag::eCOLLISION_SIDES;
    bool hit_up = collision_flags & PxControllerCollisionFlag::eCOLLISION_UP;
}

void Player::HandleMovement(float delta_time) {
    // Forward/right vectors based on yaw
    glm::vec3 forward(cos(glm::radians(yaw_)), sin(glm::radians(yaw_)), 0.0f);
    forward = glm::normalize(forward);
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 0, 1)));

    glm::vec3 move_dir(0.0f);

    if (keys_pressed_[GLFW_KEY_W]) move_dir += forward;
    if (keys_pressed_[GLFW_KEY_S]) move_dir -= forward;
    if (keys_pressed_[GLFW_KEY_A]) move_dir -= right;
    if (keys_pressed_[GLFW_KEY_D]) move_dir += right;

    if (glm::length(move_dir) > 0.0f) {
        move_dir = glm::normalize(move_dir);
    }

    float speed = is_sprinting_ ? sprint_speed_ : move_speed_;
    velocity_.x = move_dir.x * speed;
    velocity_.y = move_dir.y * speed;
}

void Player::ProcessKeyboard(int key, int action) {
    if (key < 0 || key >= 1024) return;

    if (action == GLFW_PRESS) {
        keys_pressed_[key] = true;
        if (key == GLFW_KEY_LEFT_SHIFT) is_sprinting_ = true;
    }
    else if (action == GLFW_RELEASE) {
        keys_pressed_[key] = false;
        if (key == GLFW_KEY_LEFT_SHIFT) is_sprinting_ = false;
    }
}

void Player::ProcessMouseMovement(double xpos, double ypos) {
    if (first_mouse_) {
        last_mouse_x_ = xpos;
        last_mouse_y_ = ypos;
        first_mouse_ = false;
        return;
    }

    double xoffset = (xpos - last_mouse_x_) * mouse_sensitivity_;
    double yoffset = (last_mouse_y_ - ypos) * mouse_sensitivity_;

    last_mouse_x_ = xpos;
    last_mouse_y_ = ypos;

    yaw_ += xoffset;
    pitch_ += yoffset;

    if (pitch_ > 89.0f) pitch_ = 89.0f;
    if (pitch_ < -89.0f) pitch_ = -89.0f;
}

void Player::UpdateCamera() {
    if (!camera_) return;

    // Camera at eye level (1.6m above capsule bottom)
    glm::vec3 camera_pos = position_ + glm::vec3(0, 0, 1.6f);

    // Calculate look direction
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    direction.y = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    direction.z = sin(glm::radians(pitch_));

    camera_->SetPosition(camera_pos);
    camera_->SetTarget(camera_pos + glm::normalize(direction));
    camera_->SetUp(glm::vec3(0, 0, 1));
}