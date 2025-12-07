#pragma once
#include <physx/PxPhysicsAPI.h>
#include <physx/extensions/PxDefaultStreams.h>
#include <physx/cooking/PxCooking.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>
using namespace physx;

// Character controller configuration
struct CharacterControllerConfig {
    // Capsule dimensions
    float radius = 0.4f;
    float height = 1.8f;

    // Position
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 2.0f);

    // Movement parameters
    float step_offset = 0.3f;           // Maximum height of obstacles the character can step over
    float slope_limit = 45.0f;          // Maximum slope angle in degrees (0-90)
    float contact_offset = 0.1f;        // Contact offset for collision detection
    float max_jump_height = 2.0f;       // Maximum jump height

    // Material properties
    float static_friction = 0.5f;
    float dynamic_friction = 0.5f;
    float restitution = 0.0f;           // Bounciness (0 = no bounce, 1 = perfect bounce)

    // Climbing mode
    PxCapsuleClimbingMode::Enum climbing_mode = PxCapsuleClimbingMode::eEASY;

    // Non-walkable mode - what happens when character stands on steep slopes
    PxControllerNonWalkableMode::Enum non_walkable_mode = PxControllerNonWalkableMode::ePREVENT_CLIMBING_AND_FORCE_SLIDING;

    // Density (for push forces)
    float density = 10.0f;

    // Scale coefficient for underlying kinematic actor
    float scale_coeff = 0.8f;

    // Volume growth (padding)
    float volume_growth = 0.5f;

    // Up direction (for custom gravity)
    glm::vec3 up_direction = glm::vec3(0.0f, 0.0f, 1.0f);
};

// PhysX Physics Manager (Singleton)
class PhysicsManager {
public:
    static PhysicsManager& Instance() {
        static PhysicsManager instance;
        return instance;
    }

    bool Initialize();
    void Shutdown();
    void Update(float delta_time);

    // Create capsule controller with full configuration
    PxController* CreateCapsuleController(const CharacterControllerConfig& config);

    // Create static box collider
    PxRigidStatic* CreateStaticBox(const glm::vec3& position, const glm::vec3& half_extents);

    // Create material with custom properties
    PxMaterial* CreateMaterial(float static_friction, float dynamic_friction, float restitution);

    // Collider from vertices/indices
    PxRigidStatic* CreateStaticTriangleMesh(const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices, const glm::vec3& position = glm::vec3(0.0f));

    // Collider from OBJ file - loads OBJ and creates triangle mesh collision
    PxRigidStatic* CreateCollisionFromOBJ(const std::string& obj_path, const glm::vec3& position = glm::vec3(0.0f));


    // Getters
    PxControllerManager* GetControllerManager() { return controller_manager_; }
    PxMaterial* GetDefaultMaterial() { return default_material_; }
    PxScene* GetScene() { return scene_; }

private:
    PhysicsManager() = default;
    ~PhysicsManager() = default;

    PhysicsManager(const PhysicsManager&) = delete;
    PhysicsManager& operator=(const PhysicsManager&) = delete;

    PxDefaultAllocator allocator_;
    PxDefaultErrorCallback error_callback_;
    PxFoundation* foundation_ = nullptr;
    PxPhysics* physics_ = nullptr;
    PxDefaultCpuDispatcher* dispatcher_ = nullptr;
    PxScene* scene_ = nullptr;
    PxMaterial* default_material_ = nullptr;
    PxControllerManager* controller_manager_ = nullptr;
};

// Helper functions
inline PxVec3 ToPxVec3(const glm::vec3& v) {
    return PxVec3(v.x, v.y, v.z);
}

inline glm::vec3 ToGlmVec3(const PxVec3& v) {
    return glm::vec3(v.x, v.y, v.z);
}
