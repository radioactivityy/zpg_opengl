// ------------------------------------------------------------------------------
// ZPG Temp, (c)2025 Tomas Fabian, VSB-TUO, FEECS, Dept. of Computer Science
// 
// This template is provided exclusively for non-commercial educational use in the
// Computer Graphics I and II courses at VSB-TUO. Redistribution or disclosure to
// third parties in any form is strictly prohibited.
// ------------------------------------------------------------------------------
#include <iostream>
#include <stdio.h>

#include "tutorials.h"
#include "Rasteriser.h"
#include "Collider.h"

int main()
{
    try {
        // Create the rasteriser (initializes everything)
        Rasteriser rasteriser;

        // Load shader programs
        rasteriser.LoadProgram("phong.vert", "phong.frag");
        rasteriser.LoadGrassProgram("grass.vert", "grass.frag");  // Grass shader with luminance-based alpha

        // Load collision meshes using PhysicsManager (relative paths)
        PhysicsManager::Instance().CreateCollisionFromOBJ("../../data/old_house/old_house_ground_collision.obj");
        PhysicsManager::Instance().CreateCollisionFromOBJ("../../data/old_house/old_house_ground_walls_collision.obj");

        // Load house model
        auto house = rasteriser.CreateEntity("../../data/old_house/old_house.obj", "House");

        // Table to the right
        auto table = rasteriser.CreateEntity("../../data/tables/din_table.obj", "Table");
        auto& table_transform = rasteriser.GetRegistry().get<component::Transform>(table);
        table_transform.translation = glm::vec3(5, 0, 0);
        table_transform.update_model_matrix();

        // Chest
        auto chest = rasteriser.CreateEntity("../../data/chest/chest.obj", "Chest");
        auto& chest_transform = rasteriser.GetRegistry().get<component::Transform>(chest);
        chest_transform.translation = glm::vec3(5, 0, 0);

        // Grass - create multiple grass clumps around the house
        // Use a simple grid pattern with some randomization
        std::vector<glm::vec3> grass_positions = {
            // Front of house
            glm::vec3(-8, -6, 0), glm::vec3(-6, -7, 0), glm::vec3(-4, -6, 0), glm::vec3(-2, -7, 0),
            glm::vec3(0, -6, 0), glm::vec3(2, -7, 0), glm::vec3(4, -6, 0), glm::vec3(6, -7, 0),
            glm::vec3(-7, -5, 0), glm::vec3(-5, -4, 0), glm::vec3(-3, -5, 0), glm::vec3(-1, -4, 0),
            glm::vec3(1, -5, 0), glm::vec3(3, -4, 0), glm::vec3(5, -5, 0), glm::vec3(7, -4, 0),
            // Left side
            glm::vec3(-9, -3, 0), glm::vec3(-9, -1, 0), glm::vec3(-9, 1, 0), glm::vec3(-9, 3, 0),
            glm::vec3(-10, 0, 0), glm::vec3(-10, 2, 0), glm::vec3(-10, 4, 0),
            // Right side
            glm::vec3(10, -3, 0), glm::vec3(10, -1, 0), glm::vec3(10, 1, 0), glm::vec3(10, 3, 0),
            glm::vec3(11, 0, 0), glm::vec3(11, 2, 0), glm::vec3(11, 4, 0),
            // Behind house
            glm::vec3(-6, 10, 0), glm::vec3(-4, 11, 0), glm::vec3(-2, 10, 0), glm::vec3(0, 11, 0),
            glm::vec3(2, 10, 0), glm::vec3(4, 11, 0), glm::vec3(6, 10, 0),
        };

        for (size_t i = 0; i < grass_positions.size(); i++) {
            auto grass = rasteriser.CreateEntity("../../data/grass/grass.obj", "Grass" + std::to_string(i));
            rasteriser.GetRegistry().emplace<component::Grass>(grass);
            auto& grass_transform = rasteriser.GetRegistry().get<component::Transform>(grass);
            grass_transform.translation = grass_positions[i];
            // Grass mesh is tiny (0.4 units) - scale up to ~4-6 units tall
            float scale_var = 10.0f + (i % 3) * 2.0f;
            grass_transform.scale = glm::vec3(scale_var);
            grass_transform.update_model_matrix();
        }

        // Start the main loop
        return rasteriser.Show();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}


