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

        // Grass - add with Grass component for special rendering
        auto grass = rasteriser.CreateEntity("../../data/grass/grass.obj", "Grass");
        rasteriser.GetRegistry().emplace<component::Grass>(grass);  // Tag as grass for special shader
        auto& grass_transform = rasteriser.GetRegistry().get<component::Transform>(grass);
        grass_transform.translation = glm::vec3(0, -5, 0);  // Position in front of house
        grass_transform.scale = glm::vec3(5.0f);  // Scale up to be visible
        grass_transform.update_model_matrix();

        // Start the main loop
        return rasteriser.Show();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}


