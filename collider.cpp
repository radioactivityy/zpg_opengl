#include "Collider.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>



PxRigidStatic* PhysicsManager::CreateStaticTriangleMesh(const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices, const glm::vec3& position) {
    if (!physics_ || !scene_) {
        std::cerr << "Physics or scene not initialized!" << std::endl;
        return nullptr;
    }

    // Prepare PhysX mesh description
    PxTriangleMeshDesc meshDesc;
    meshDesc.points.count = static_cast<PxU32>(vertices.size());
    meshDesc.points.stride = sizeof(glm::vec3);
    meshDesc.points.data = vertices.data();

    meshDesc.triangles.count = static_cast<PxU32>(indices.size() / 3);
    meshDesc.triangles.stride = 3 * sizeof(uint32_t);
    meshDesc.triangles.data = indices.data();

    // Use PxPhysics to cook the mesh directly (no separate PxCooking object needed)
    PxTolerancesScale scale = physics_->getTolerancesScale();
    PxCookingParams cookingParams(scale);

    // Cook the mesh in memory
    PxDefaultMemoryOutputStream writeBuffer;
    PxTriangleMeshCookingResult::Enum result;

    bool success = PxCookTriangleMesh(cookingParams, meshDesc, writeBuffer, &result);

    if (!success) {
        std::cerr << "Failed to cook triangle mesh! Result: " << result << std::endl;
        return nullptr;
    }

    // Create triangle mesh from cooked data
    PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
    PxTriangleMesh* triangleMesh = physics_->createTriangleMesh(readBuffer);

    if (!triangleMesh) {
        std::cerr << "Failed to create triangle mesh from cooked data!" << std::endl;
        return nullptr;
    }

    // Create static actor
    PxTransform transform(ToPxVec3(position));
    PxRigidStatic* actor = physics_->createRigidStatic(transform);

    if (!actor) {
        std::cerr << "Failed to create rigid static actor!" << std::endl;
        triangleMesh->release();
        return nullptr;
    }

    // Create shape with triangle mesh geometry
    PxShape* shape = physics_->createShape(PxTriangleMeshGeometry(triangleMesh), *default_material_);

    if (!shape) {
        std::cerr << "Failed to create shape!" << std::endl;
        actor->release();
        triangleMesh->release();
        return nullptr;
    }

    actor->attachShape(*shape);
    shape->release();

    scene_->addActor(*actor);

    std::cout << "Triangle mesh collider created successfully!" << std::endl;
    return actor;
}

PxRigidStatic* PhysicsManager::CreateCollisionFromOBJ(const std::string& obj_path, const glm::vec3& position) {
    std::ifstream file(obj_path);
    if (!file.is_open()) {
        std::cerr << "Failed to open OBJ file for collision: " << obj_path << std::endl;
        return nullptr;
    }

    std::vector<glm::vec3> vertices;
    std::vector<uint32_t> indices;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            // Vertex position
            float x, y, z;
            iss >> x >> y >> z;
            vertices.push_back(glm::vec3(x, y, z));
        }
        else if (prefix == "f") {
            // Face - parse vertex indices (handles v, v/vt, v/vt/vn, v//vn formats)
            std::string vertex_str;
            std::vector<uint32_t> face_indices;

            while (iss >> vertex_str) {
                // Extract vertex index (before first '/')
                size_t slash_pos = vertex_str.find('/');
                std::string index_str = (slash_pos != std::string::npos)
                    ? vertex_str.substr(0, slash_pos)
                    : vertex_str;
                
                int index = std::stoi(index_str);
                // OBJ indices are 1-based, convert to 0-based
                face_indices.push_back(static_cast<uint32_t>(index > 0 ? index - 1 : vertices.size() + index));
            }

            // Triangulate face (fan triangulation for convex polygons)
            for (size_t i = 1; i + 1 < face_indices.size(); ++i) {
                indices.push_back(face_indices[0]);
                indices.push_back(face_indices[i]);
                indices.push_back(face_indices[i + 1]);
            }
        }
    }

    file.close();

    if (vertices.empty() || indices.empty()) {
        std::cerr << "OBJ file has no geometry: " << obj_path << std::endl;
        return nullptr;
    }

    std::cout << "Loaded collision OBJ: " << vertices.size() << " vertices, "
              << indices.size() / 3 << " triangles from " << obj_path << std::endl;

    return CreateStaticTriangleMesh(vertices, indices, position);
}

bool PhysicsManager::Initialize() {
    foundation_ = PxCreateFoundation(PX_PHYSICS_VERSION, allocator_, error_callback_);
    if (!foundation_) {
        std::cerr << "PxCreateFoundation failed!" << std::endl;
        return false;
    }

    physics_ = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation_, PxTolerancesScale(), true, nullptr);
    if (!physics_) {
        std::cerr << "PxCreatePhysics failed!" << std::endl;
        return false;
    }

    // CRITICAL: Initialize PhysX extensions (required for character controllers)
    if (!PxInitExtensions(*physics_, nullptr)) {
        std::cerr << "PxInitExtensions failed!" << std::endl;
        return false;
    }

    PxSceneDesc scene_desc(physics_->getTolerancesScale());
    scene_desc.gravity = PxVec3(0.0f, 0.0f, -9.81f);

    dispatcher_ = PxDefaultCpuDispatcherCreate(2);
    scene_desc.cpuDispatcher = dispatcher_;
    scene_desc.filterShader = PxDefaultSimulationFilterShader;

    scene_ = physics_->createScene(scene_desc);
    if (!scene_) {
        std::cerr << "createScene failed!" << std::endl;
        return false;
    }

    default_material_ = physics_->createMaterial(0.5f, 0.5f, 0.1f);

    // Create controller manager
    controller_manager_ = PxCreateControllerManager(*scene_);
    if (!controller_manager_) {
        std::cerr << "PxCreateControllerManager failed!" << std::endl;
        return false;
    }

    std::cout << "PhysX initialized successfully!" << std::endl;
    return true;
}

void PhysicsManager::Shutdown() {
    if (!foundation_) {
        return;
    }

    if (controller_manager_) {
        controller_manager_->release();
        controller_manager_ = nullptr;
    }
    if (scene_) {
        scene_->release();
        scene_ = nullptr;
    }
    if (dispatcher_) {
        dispatcher_->release();
        dispatcher_ = nullptr;
    }

    // Close extensions
    PxCloseExtensions();

    if (physics_) {
        physics_->release();
        physics_ = nullptr;
    }
    if (foundation_) {
        foundation_->release();
        foundation_ = nullptr;
    }

    std::cout << "PhysX shut down." << std::endl;
}

void PhysicsManager::Update(float delta_time) {
    if (scene_) {
        scene_->simulate(delta_time);
        scene_->fetchResults(true);
    }
}

PxController* PhysicsManager::CreateCapsuleController(const CharacterControllerConfig& config) {
    if (!controller_manager_) {
        std::cerr << "Controller manager not initialized!" << std::endl;
        return nullptr;
    }

    // Setup capsule controller descriptor
    PxCapsuleControllerDesc desc;

    // Position
    desc.position = PxExtendedVec3(config.position.x, config.position.y, config.position.z);

    // Capsule dimensions
    desc.radius = config.radius;
    desc.height = config.height;

    // Use default material (custom materials were causing the issue!)
    desc.material = default_material_;

    // Movement parameters
    desc.stepOffset = config.step_offset;
    desc.slopeLimit = cosf(glm::radians(config.slope_limit));
    desc.contactOffset = config.contact_offset;

    // Up direction
    desc.upDirection = ToPxVec3(config.up_direction);

    // Climbing and slope behavior
    desc.climbingMode = config.climbing_mode;
    desc.nonWalkableMode = config.non_walkable_mode;

    // Other parameters
    desc.density = config.density;
    desc.scaleCoeff = config.scale_coeff;
    desc.volumeGrowth = config.volume_growth;
    desc.invisibleWallHeight = 0.0f;
    desc.maxJumpHeight = config.max_jump_height;

    // Callbacks (can be nullptr)
    desc.reportCallback = nullptr;
    desc.behaviorCallback = nullptr;
    desc.userData = nullptr;

    // Validate
    if (!desc.isValid()) {
        std::cerr << "Invalid capsule controller descriptor!" << std::endl;
        return nullptr;
    }

    // Create controller
    PxController* controller = controller_manager_->createController(desc);

    if (controller) {
        std::cout << "Character controller created at position ("
            << config.position.x << ", "
            << config.position.y << ", "
            << config.position.z << ")" << std::endl;
    }
    else {
        std::cerr << "Failed to create controller!" << std::endl;
    }

    return controller;
}

PxRigidStatic* PhysicsManager::CreateStaticBox(const glm::vec3& position, const glm::vec3& half_extents) {
    PxTransform transform(ToPxVec3(position));
    PxRigidStatic* actor = PxCreateStatic(*physics_, transform,
        PxBoxGeometry(ToPxVec3(half_extents)), *default_material_);

    if (actor && scene_) {
        scene_->addActor(*actor);
    }
    return actor;
}

PxMaterial* PhysicsManager::CreateMaterial(float static_friction, float dynamic_friction, float restitution) {
    return physics_->createMaterial(static_friction, dynamic_friction, restitution);
}