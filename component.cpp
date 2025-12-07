#include "component.h"

namespace component {
    void Transform::update_model_matrix() {
        // Update local transformation matrix
        local_model_matrix = glm::translate(glm::mat4(1.0f), translation)
            * glm::eulerAngleZYX(rotation.z, rotation.y, rotation.x)
            * glm::scale(glm::mat4(1.0f), scale);
    }

    void Transform::update_world_matrix(const glm::mat4& parent_world_matrix) {
        // Combine parent's world matrix with this entity's local matrix
        world_model_matrix = parent_world_matrix * local_model_matrix;
    }

    glm::mat4 Transform::get_world_matrix(entt::registry& registry, entt::entity entity) {
        // Update local matrix first
        update_model_matrix();

        // Check if this entity has a parent
        if (registry.all_of<Children>(entity)) {
            auto& children = registry.get<Children>(entity);

            if (children.has_parent() && registry.valid(children.parent)) {
                // Recursively get parent's world matrix
                auto& parent_transform = registry.get<Transform>(children.parent);
                glm::mat4 parent_world = parent_transform.get_world_matrix(registry, children.parent);

                // Combine parent's world matrix with our local matrix
                world_model_matrix = parent_world * local_model_matrix;
                return world_model_matrix;
            }
        }

        // No parent - world matrix = local matrix
        world_model_matrix = local_model_matrix;
        return world_model_matrix;
    }
}