#pragma once
#pragma once
#include <glm/glm.hpp>
#include <physx/PxPhysicsAPI.h>
#include "Camera.h"
#include "Collider.h"

using namespace physx;

class Player {
public:
    Player(Camera* camera);
    ~Player();

    // Initialize with configuration
    void Initialize(const CharacterControllerConfig& config);

    // Or use default configuration at a position
    void Initialize(const glm::vec3& spawn_position);

    void Update(float delta_time);

    // Input handling
    void ProcessKeyboard(int key, int action);
    void ProcessMouseMovement(double xpos, double ypos);

    // Getters/Setters
    glm::vec3 GetPosition() const { return position_; }
    void SetPosition(const glm::vec3& pos);

    // Movement parameters
    void SetMoveSpeed(float speed) { move_speed_ = speed; }
    void SetSprintSpeed(float speed) { sprint_speed_ = speed; }
    void SetJumpForce(float force) { jump_force_ = force; }
    void SetMouseSensitivity(float sensitivity) { mouse_sensitivity_ = sensitivity; }
    void SetInitialYaw(float yaw) { yaw_ = yaw; UpdateCamera(); }
    void SetInitialPitch(float pitch) { pitch_ = pitch; UpdateCamera(); }

private:
    Camera* camera_;
    PxController* controller_ = nullptr;

    glm::vec3 position_;
    glm::vec3 velocity_;

    float move_speed_ = 5.0f;
    float sprint_speed_ = 10.0f;
    float jump_force_ = 5.0f;
    float mouse_sensitivity_ = 0.1f;

    float yaw_ = 0.0f;
    float pitch_ = 0.0f;

    bool keys_pressed_[1024] = { false };
    bool is_grounded_ = false;
    bool is_sprinting_ = false;
    bool first_mouse_ = true;
    double last_mouse_x_ = 0.0;
    double last_mouse_y_ = 0.0;

    void UpdateCamera();
    void HandleMovement(float delta_time);
};