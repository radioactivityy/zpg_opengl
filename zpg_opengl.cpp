// ------------------------------------------------------------------------------
// ZPG Temp, (c)2025 Tomas Fabian, VSB-TUO, FEECS, Dept. of Computer Science
// 
// This template is provided exclusively for non-commercial educational use in the
// Computer Graphics I and II courses at VSB-TUO. Redistribution or disclosure to
// third parties in any form is strictly prohibited.
// ------------------------------------------------------------------------------
#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <ctime>

#include "tutorials.h"
#include "Rasteriser.h"
#include "Collider.h"

int main()
{
    // Seed random number generator for grass variation
    srand(static_cast<unsigned>(time(nullptr)));

    try {
        // Create the rasteriser (initializes everything)
        Rasteriser rasteriser;

        // Load shader programs
        rasteriser.LoadProgram("phong.vert", "phong.frag");
        rasteriser.LoadGrassProgram("grass.vert", "grass.frag");  // Grass shader with luminance-based alpha
        rasteriser.LoadShadowProgram("shadow.vert", "shadow.frag");  // Shadow mapping shaders
        rasteriser.LoadSkyboxProgram("skybox.vert", "skybox.frag");  // Environmental background shader

        // Initialize shadow mapping
        rasteriser.InitShadowDepthbuffer();

        // Load skybox/environment background texture
        // Path: ../../data/skybox/background.png (relative to build output folder x64/Debug/)
        rasteriser.LoadSkyboxTexture("../../data/skybox/background.png");

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

        // Grass - create dense grass field around the house
        // Generate grass positions in a grid with some randomness
        std::vector<glm::vec3> grass_positions;

        // Create a dense grid of grass around the house
        for (float x = -12; x <= 12; x += 1.5f) {
            for (float y = -10; y <= 14; y += 1.5f) {
                // Skip areas inside/under the house (approximate house footprint)
                if (x > -5 && x < 8 && y > -2 && y < 8) continue;

                // Add some randomness to position
                float rx = (rand() % 100) / 100.0f - 0.5f;
                float ry = (rand() % 100) / 100.0f - 0.5f;
                grass_positions.push_back(glm::vec3(x + rx, y + ry, 0));
            }
        }

        for (size_t i = 0; i < grass_positions.size(); i++) {
            auto grass = rasteriser.CreateEntity("../../data/grass/grass.obj", "Grass" + std::to_string(i));
            rasteriser.GetRegistry().emplace<component::Grass>(grass);
            auto& grass_transform = rasteriser.GetRegistry().get<component::Transform>(grass);
            grass_transform.translation = grass_positions[i];
            // Vary scale for natural look (grass mesh is ~0.4 units)
            float scale_var = 2.5f + (rand() % 100) / 100.0f * 1.5f;
            grass_transform.scale = glm::vec3(scale_var);
            // Random rotation around Z axis for variety
            grass_transform.rotation = glm::vec3(0, 0, (rand() % 360) * 3.14159f / 180.0f);
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


