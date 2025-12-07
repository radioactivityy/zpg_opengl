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

int main()
{
    try {
        // Create the rasteriser (initializes everything)
        Rasteriser rasteriser;

        // Load shader program
        rasteriser.LoadProgram("phong.vert", "phong.frag");

        // In your initialization or scene setup code:
        rasteriser.AddCollisionFromOBJ("C:/Users/demirgul/Downloads/zpg_opengl_template/zpg_opengl_template/data/old_house/old_house_ground_collision.obj");
        rasteriser.AddCollisionFromOBJ("C:/Users/demirgul/Downloads/zpg_opengl_template/zpg_opengl_template/data/old_house/old_house_ground_walls_collision.obj");


        // Optional: Load some entities to see in the scene
        // Create a ground/floor entity

        // Optional: Load your models
        auto house = rasteriser.CreateEntity("../../data/old_house/old_house.obj", "House");
		

        //// Table to the right
        auto table = rasteriser.CreateEntity("../../data/tables/din_table.obj", "Table");
        auto& table_transform = rasteriser.GetRegistry().get<component::Transform>(table);
        table_transform.translation = glm::vec3(5, 0, 0);
        table_transform.update_model_matrix();

        // Chest - just rotate, no translation
        auto chest = rasteriser.CreateEntity("../../data/chest/chest.obj", "Chest");
        auto& chest_transform = rasteriser.GetRegistry().get<component::Transform>(chest);
        chest_transform.translation = glm::vec3(5, 0, 0);
        // Start the main loop
        return rasteriser.Show();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}


