#pragma once
#include "glutils.h"
#include <glm/gtx/euler_angles.hpp>
// In components.h
namespace component {
    struct Transform {
        glm::vec3 translation{ 0.0f };
        glm::vec3 rotation{ 0.0f };
        glm::vec3 scale{ 1.0f };
        glm::mat4 local_model_matrix{ 1.0f };
        glm::mat4 world_model_matrix{ 1.0f };

        // Update local transformation matrix from translation, rotation, scale
        void update_model_matrix();

        // Update world matrix by combining with parent's world matrix
        void update_world_matrix(const glm::mat4& parent_world_matrix);

        // Recursively calculate world matrix by going up the parent chain
        glm::mat4 get_world_matrix(entt::registry& registry, entt::entity entity);
    };

    struct Mesh {
        std::vector<GLMesh> gl_meshes;  // List of sub-meshes
    };

    struct Name {
        std::string value;
    };

    // Parent component - entity has children
    struct Parent {
        std::vector<entt::entity> children;

        bool has_children() const {
            return !children.empty();
        }

        void add_child(entt::entity child) {
            children.push_back(child);
        }

        void remove_child(entt::entity child) {
            children.erase(
                std::remove(children.begin(), children.end(), child),
                children.end()
            );
        }
    };

    // Children component - entity has a parent
    struct Children {
        entt::entity parent{ entt::null };

        bool has_parent() const {
            return parent != entt::null;
        }
    };

    // Tag component for grass entities (use grass shader with wind animation)
    struct Grass {};
}