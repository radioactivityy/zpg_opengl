#include "Rasteriser.h"
#include <iostream>
#include <assert.h>
#include <filesystem>
#include <cmath>
#include <algorithm>

void Rasteriser::AddCollisionFromOBJ(const std::string& obj_path, const glm::vec3& position) {
    // Only allow the two specific files
    std::filesystem::path p(obj_path);
    std::string filename = p.filename().string();
    if (filename != "old_house_ground_collision.obj" && filename != "old_house_ground_walls_collision.obj") {
        std::cout << "Collision only supported for specific files." << std::endl;
        return;
    }

    // Load mesh using MeshLoader
    std::vector<std::shared_ptr<TriangularMesh>> meshes;
    _mesh_loader->LoadTriangularMesh(obj_path, meshes);

    for (const auto& mesh : meshes) {
        // Extract vertices
        std::vector<glm::vec3> vertices;
        const Vertex* verts = static_cast<const Vertex*>(mesh->vertex_buffer());
        size_t vcount = mesh->vertex_buffer_count();

        for (size_t i = 0; i < vcount; ++i) {
            vertices.push_back(verts[i].position);
        }

        // Extract indices from Triangle structures
        std::vector<uint32_t> indices;
        const Triangle* triangles = static_cast<const Triangle*>(mesh->index_buffer());
        size_t triangle_count = mesh->index_buffer_count();

        // Each Triangle contains 3 indices - extract them
        for (size_t i = 0; i < triangle_count; ++i) {
            // Triangle struct typically has an array or three separate members
            // Access the indices (assuming Triangle has an indices array or similar)
            const uint32_t* tri_indices = reinterpret_cast<const uint32_t*>(&triangles[i]);
            indices.push_back(tri_indices[0]);
            indices.push_back(tri_indices[1]);
            indices.push_back(tri_indices[2]);
        }

        std::cout << "Creating collision mesh with " << vertices.size()
            << " vertices and " << indices.size() << " indices ("
            << triangle_count << " triangles)" << std::endl;

        // Create triangle mesh collider
        PhysicsManager::Instance().CreateStaticTriangleMesh(vertices, indices, position);
    }
}


//Rasteriser::Rasteriser() {
//    InitOpenGLContext();
//    _mesh_loader = std::make_unique<MeshLoader>();
//}
Rasteriser::Rasteriser() {
    InitOpenGLContext();
    _mesh_loader = std::make_unique<MeshLoader>();

    // Get viewport FIRST
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    // Initialize camera with proper values
    camera_ = std::make_unique<Camera>();
    camera_->SetWidth(viewport[2]);
    camera_->SetHeight(viewport[3]);
    camera_->SetFOV(45.0);

    camera_->SetUp(glm::vec3(0, 0, 1));

    // Initialize camera position using orbit settings
    UpdateOrbitCamera();

    PhysicsManager::Instance().Initialize();

  
  glfwSetWindowUserPointer(_window, this);
  glfwSetKeyCallback(_window, key_callback);
  glfwSetCursorPosCallback(_window, mouse_callback);
  glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // NOTE: Don't create a large ground plane - let the collision mesh handle ground
    // The player should fall if they walk off the house's ground collision

  // Enable first-person player controller
  // Spawn in front of house (Y = -8), slightly above ground (Z = 3) so player falls
  player_ = std::make_unique<Player>(camera_.get());
  player_->Initialize(glm::vec3(0, -8, 3));  // In front of house, above ground
  player_->SetInitialYaw(90.0f);  // Face north toward the house (0,0,0)
}

// In Rasteriser.cpp
Rasteriser::~Rasteriser() {
    // Destroy player first (releases controller)
    player_.reset();

    // Then shutdown PhysX
    PhysicsManager::Instance().Shutdown();
}


int Rasteriser::InitOpenGLContext() {
    glfwSetErrorCallback(glfw_callback);

    if (!glfwInit())
    {
        return(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 8);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);

    _window = glfwCreateWindow(width_, height_, "PG2 OpenGL", nullptr, nullptr);
    if (!_window)
    {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwSetFramebufferSizeCallback(_window, framebuffer_resize_callback);
    glfwMakeContextCurrent(_window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        if (!gladLoadGL())
        {
            return EXIT_FAILURE;
        }
    }

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(gl_callback, nullptr);

    printf("OpenGL %s, ", glGetString(GL_VERSION));
    printf("%s", glGetString(GL_RENDERER));
    printf(" (%s)\n", glGetString(GL_VENDOR));
    printf("GLSL %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    glEnable(GL_MULTISAMPLE);

    // map from the range of NDC coordinates <-1.0, 1.0>^2 to <0, width> x <0, height>
    glViewport(0, 0, width_, height_);
    glEnable(GL_FRAMEBUFFER_SRGB);
    // GL_LOWER_LEFT (OpenGL) or GL_UPPER_LEFT (DirectX, Windows) and GL_NEGATIVE_ONE_TO_ONE or GL_ZERO_TO_ONE
    glClipControl(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
}



entt::entity Rasteriser::CreateEntity(const std::string& mesh_file, const std::string& name, entt::entity parent) {
    auto entity = registry_.create();

    // Add Transform component
    registry_.emplace<component::Transform>(entity);

    // Add Name component
    registry_.emplace<component::Name>(entity, name);

    // Add Mesh component and load meshes
    auto& mesh_component = registry_.emplace<component::Mesh>(entity);
    LoadMesh(mesh_file, mesh_component.gl_meshes);

    // Set up parent-child relationship if parent is provided
    if (parent != entt::null && registry_.valid(parent)) {
        // Add Children component to this entity
        auto& children_comp = registry_.emplace<component::Children>(entity);
        children_comp.parent = parent;

        // Add or get Parent component from parent entity
        if (!registry_.all_of<component::Parent>(parent)) {
            registry_.emplace<component::Parent>(parent);
        }
        auto& parent_comp = registry_.get<component::Parent>(parent);
        parent_comp.add_child(entity);
    }

    return entity;
}

int Rasteriser::LoadMesh(const std::string& file_mame, std::vector<GLMesh> & gl_meshes) {

    //https://mrl.cs.vsb.cz/people/fabian/pg1/6887.zip

    meshes_.clear();

    _mesh_loader->LoadTriangularMesh(file_mame, meshes_);
   
    for (const auto& mesh : meshes_) {
        auto vertices = mesh->vertex_buffer();
        const size_t vertex_buffer_size = mesh->vertex_buffer_size();
        const size_t vertex_buffer_count = mesh->vertex_buffer_count();
        const GLsizei vertex_stride = vertices[0].size();
        assert(vertex_stride == vertex_buffer_size / vertex_buffer_count);

        auto indices = mesh->index_buffer();
        const size_t index_buffer_size = mesh->index_buffer_size();
        const size_t index_buffer_count = mesh->index_buffer_count();

        // ===== CORRECTED DIAGNOSTIC CODE =====
        std::cout << "\n=== Vertex Material Index Debug ===" << std::endl;
        std::cout << "Mesh has " << vertex_buffer_count << " vertices" << std::endl;
        std::cout << "Material indices in vertices: ";

        // Cast the void* to Vertex* so we can access the data
        const Vertex* vertex_array = static_cast<const Vertex*>(vertices);
        for (size_t i = 0; i < std::min(vertex_buffer_count, size_t(20)); ++i) {
            std::cout << vertex_array[i].mat_idx.x << " ";
        }
        if (vertex_buffer_count > 20) {
            std::cout << "...";
        }
        std::cout << std::endl;
        std::cout << "Current materials array size: " << materials_.size() << std::endl;
        std::cout << "====================================\n" << std::endl;
        // ===== END OF DIAGNOSTIC CODE =====

        GLuint vao = 0;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        GLuint vbo = 0;
        glGenBuffers(1, &vbo); // generate vertex buffer object (one of OpenGL objects) and get the unique ID corresponding to that buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo); // bind the newly created buffer to the GL_ARRAY_BUFFER target
        glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size, vertices, GL_STATIC_DRAW); // copies the previously defined vertex data into the buffer's memory
        // vertex position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, GLsizei(vertex_stride), 0);
        glEnableVertexAttribArray(0);
        // vertex normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, GLsizei(vertex_stride), (void*)offsetof(Vertex, normal));
        glEnableVertexAttribArray(1);
        //vector tangent
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, GLsizei(vertex_stride), (void*)offsetof(Vertex, tangent));
        glEnableVertexAttribArray(2);

        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, GLsizei(vertex_stride), (void*)offsetof(Vertex, tex_coord));
        glEnableVertexAttribArray(3);

        glVertexAttribIPointer(4, 1, GL_INT, GLsizei(vertex_stride), (void*)offsetof(Vertex, mat_idx));
        glEnableVertexAttribArray(4);
        auto material = mesh->material();
        std::cout << "=== Material Debug: " << material->name() << " ===" << std::endl;
        std::cout << "Has diffuse_map: " << (material->diffuse_map != nullptr) << std::endl;
        if (material->diffuse_map) {
            std::cout << "Diffuse map dimensions: " << material->diffuse_map->width()
                << "x" << material->diffuse_map->height() << std::endl;
            std::cout << "Diffuse map data pointer: " << material->diffuse_map->data() << std::endl;
        }
        std::cout << "Has normal_map: " << (material->normal_map != nullptr) << std::endl;
        std::cout << "Has specular_map: " << (material->specular_map != nullptr) << std::endl;
        std::cout << "=========================" << std::endl;

        GLMaterial gl_mat{};

        // Set diffuse color - use diffuse_color from material
        Color3f diffuse_color_val = material->diffuse_color.value_or(Color3f::gray50);
        gl_mat.diffuse = diffuse_color_val;

        // Set RMA values - convert specular to roughness, assume non-metallic
        Color3f specular_color_val = material->specular_color.value_or(Color3f::gray50);
        float specular_intensity = specular_color_val.luminance();
        float roughness = 1.0f - (specular_intensity * 0.5f); // Convert specular to roughness
        float metalness = 0.0f; // Most OBJ materials are non-metallic
        float ao = 1.0f; // Default AO
        gl_mat.rma = Color3f({ roughness, metalness, ao });

        // Set normal - default normal
        gl_mat.normal = Color3f({ 0.5f, 0.5f, 1.0f });


        // Create bindless textures

        // Diffuse texture
        if (material->diffuse_map) {
            GLuint tex_id;
            CreateBindlessTexture(tex_id, gl_mat.tex_diffuse_handle,
                material->diffuse_map->width(), material->diffuse_map->height(),
                material->diffuse_map->data(), 0);
            std::cout << "Created diffuse texture handle: " << gl_mat.tex_diffuse_handle << std::endl;
        }
        else {
            gl_mat.tex_diffuse_handle = 0;
            std::cout << "No diffuse texture, using color: ("
                << diffuse_color_val.data[0] << ", "
                << diffuse_color_val.data[1] << ", "
                << diffuse_color_val.data[2] << ")" << std::endl;
        }

        // Normal map
        if (material->normal_map) {
            GLuint tex_id;
            CreateBindlessTexture(tex_id, gl_mat.tex_normal_handle,
                material->normal_map->width(), material->normal_map->height(),
                material->normal_map->data(), 0);
            std::cout << "Created normal texture handle: " << gl_mat.tex_normal_handle << std::endl;
        }
        else {
            gl_mat.tex_normal_handle = 0;
        }

        // RMA texture - use specular map if available, or roughness/metallic maps
        if (material->specular_map) {
            GLuint tex_id;
            CreateBindlessTexture(tex_id, gl_mat.tex_rma_handle,
                material->specular_map->width(), material->specular_map->height(),
                material->specular_map->data(), 0);
            std::cout << "Created RMA texture handle from specular map: " << gl_mat.tex_rma_handle << std::endl;
        }
        else if (material->roughness_map) {
            GLuint tex_id;
            CreateBindlessTexture(tex_id, gl_mat.tex_rma_handle,
                material->roughness_map->width(), material->roughness_map->height(),
                material->roughness_map->data(), 0);
            std::cout << "Created RMA texture handle from roughness map: " << gl_mat.tex_rma_handle << std::endl;
        }
        else if (material->metallic_map) {
            GLuint tex_id;
            CreateBindlessTexture(tex_id, gl_mat.tex_rma_handle,
                material->metallic_map->width(), material->metallic_map->height(),
                material->metallic_map->data(), 0);
            std::cout << "Created RMA texture handle from metallic map: " << gl_mat.tex_rma_handle << std::endl;
        }
        else {
            gl_mat.tex_rma_handle = 0;
        }

        // Debug output for material properties
        std::cout << "Material properties - Roughness: " << roughness
            << ", Metalness: " << metalness
            << ", Specular: " << specular_intensity << std::endl;

        materials_.push_back(gl_mat);

        GLuint ebo = 0;
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_size, indices, GL_STATIC_DRAW);
        gl_meshes.push_back(GLMesh{ vao, vbo, ebo, mesh });
    }

    if (materials_ssbo == 0) {
        glGenBuffers(1, &materials_ssbo);
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, materials_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
        materials_.size() * sizeof(GLMaterial),
        materials_.data(),
        GL_DYNAMIC_DRAW);  // Consider using DYNAMIC_DRAW if you update materials
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, materials_ssbo);


    std::cout << "Created materials SSBO with " << materials_.size() << " materials, handle: " << materials_ssbo << std::endl;
    return 0;
}

int Rasteriser::LoadProgram(const std::string& vs_file_name, const std::string& fs_file_name)
{
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    std::vector<char> shader_source;
    if (LoadShader(vs_file_name, shader_source) == S_OK)
    {
        const char* tmp = static_cast<const char*>(&shader_source[0]);
        glShaderSource(vertex_shader, 1, &tmp, nullptr);
        glCompileShader(vertex_shader);
    }
    CheckShader(vertex_shader);

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    if (LoadShader(fs_file_name, shader_source) == S_OK)
    {
        const char* tmp = static_cast<const char*>(&shader_source[0]);
        glShaderSource(fragment_shader, 1, &tmp, nullptr);
        glCompileShader(fragment_shader);
    }
    CheckShader(fragment_shader);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    // TODO check linking
    glUseProgram(shader_program);
    default_shader_program_ = shader_program;

    return 0;

}

int Rasteriser::LoadGrassProgram(const std::string& vs_file_name, const std::string& fs_file_name)
{
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    std::vector<char> shader_source;
    if (LoadShader(vs_file_name, shader_source) == S_OK)
    {
        const char* tmp = static_cast<const char*>(&shader_source[0]);
        glShaderSource(vertex_shader, 1, &tmp, nullptr);
        glCompileShader(vertex_shader);
    }
    CheckShader(vertex_shader);

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    if (LoadShader(fs_file_name, shader_source) == S_OK)
    {
        const char* tmp = static_cast<const char*>(&shader_source[0]);
        glShaderSource(fragment_shader, 1, &tmp, nullptr);
        glCompileShader(fragment_shader);
    }
    CheckShader(fragment_shader);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    grass_shader_program_ = shader_program;

    std::cout << "Grass shader program loaded: " << grass_shader_program_ << std::endl;
    return 0;
}

int Rasteriser::LoadShadowProgram(const std::string& vs_file_name, const std::string& fs_file_name)
{
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    std::vector<char> shader_source;
    if (LoadShader(vs_file_name, shader_source) == S_OK)
    {
        const char* tmp = static_cast<const char*>(&shader_source[0]);
        glShaderSource(vertex_shader, 1, &tmp, nullptr);
        glCompileShader(vertex_shader);
    }
    CheckShader(vertex_shader);

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    if (LoadShader(fs_file_name, shader_source) == S_OK)
    {
        const char* tmp = static_cast<const char*>(&shader_source[0]);
        glShaderSource(fragment_shader, 1, &tmp, nullptr);
        glCompileShader(fragment_shader);
    }
    CheckShader(fragment_shader);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    shadow_program_ = shader_program;

    std::cout << "Shadow shader program loaded: " << shadow_program_ << std::endl;
    return 0;
}

void Rasteriser::InitShadowDepthbuffer()
{
    // Create texture to hold depth values from light's perspective
    glGenTextures(1, &tex_shadow_map_);
    glBindTexture(GL_TEXTURE_2D, tex_shadow_map_);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadow_width_, shadow_height_,
                 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    // Areas outside the light's frustum will be lit (white border)
    const float border_color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);

    glBindTexture(GL_TEXTURE_2D, 0);

    // Create framebuffer for shadow pass
    glGenFramebuffers(1, &fbo_shadow_map_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_shadow_map_);

    // Attach texture as depth attachment
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex_shadow_map_, 0);

    // We don't need color buffer for depth pass
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR: Shadow framebuffer is not complete!" << std::endl;
    } else {
        std::cout << "Shadow framebuffer initialized: " << shadow_width_ << "x" << shadow_height_ << std::endl;
    }

    // Bind default framebuffer back
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int Rasteriser::LoadSkyboxProgram(const std::string& vs_file_name, const std::string& fs_file_name)
{
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    std::vector<char> shader_source;
    if (LoadShader(vs_file_name, shader_source) == S_OK)
    {
        const char* tmp = static_cast<const char*>(&shader_source[0]);
        glShaderSource(vertex_shader, 1, &tmp, nullptr);
        glCompileShader(vertex_shader);
    }
    CheckShader(vertex_shader);

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    if (LoadShader(fs_file_name, shader_source) == S_OK)
    {
        const char* tmp = static_cast<const char*>(&shader_source[0]);
        glShaderSource(fragment_shader, 1, &tmp, nullptr);
        glCompileShader(fragment_shader);
    }
    CheckShader(fragment_shader);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    skybox_shader_program_ = shader_program;

    // Create VAO for fullscreen triangle (uses gl_VertexID, no actual vertex data needed)
    glGenVertexArrays(1, &skybox_vao_);

    std::cout << "Skybox shader program loaded: " << skybox_shader_program_ << std::endl;
    return 0;
}

void Rasteriser::LoadSkyboxTexture(const std::string& texture_path)
{
    // Load the texture using FreeImage through the Texture class
    Texture texture = Texture3u(texture_path);

    if (texture.width() == 0 || texture.height() == 0) {
        std::cout << "ERROR: Failed to load skybox texture from: " << texture_path << std::endl;
        return;
    }

    std::cout << "Loading skybox texture: " << texture_path << std::endl;
    std::cout << "  Dimensions: " << texture.width() << "x" << texture.height() << std::endl;

    // Create bindless texture for skybox
    CreateBindlessTexture(skybox_texture_, skybox_texture_handle_,
        texture.width(), texture.height(), texture.data(), 0);

    if (skybox_texture_handle_ != 0) {
        std::cout << "Skybox texture loaded successfully, handle: " << skybox_texture_handle_ << std::endl;
    }
    else {
        std::cout << "ERROR: Failed to create skybox texture handle!" << std::endl;
    }
}

int Rasteriser::LoadRainProgram(const std::string& vs_file_name, const std::string& fs_file_name)
{
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    std::vector<char> shader_source;
    if (LoadShader(vs_file_name, shader_source) == S_OK)
    {
        const char* tmp = static_cast<const char*>(&shader_source[0]);
        glShaderSource(vertex_shader, 1, &tmp, nullptr);
        glCompileShader(vertex_shader);
    }
    CheckShader(vertex_shader);

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    if (LoadShader(fs_file_name, shader_source) == S_OK)
    {
        const char* tmp = static_cast<const char*>(&shader_source[0]);
        glShaderSource(fragment_shader, 1, &tmp, nullptr);
        glCompileShader(fragment_shader);
    }
    CheckShader(fragment_shader);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    rain_shader_program_ = shader_program;

    std::cout << "Rain shader program loaded: " << rain_shader_program_ << std::endl;
    return 0;
}

void Rasteriser::InitRainParticles()
{
    rain_particles_.resize(RAIN_PARTICLE_COUNT);

    // Initialize particles with random positions around the camera
    for (int i = 0; i < RAIN_PARTICLE_COUNT; ++i) {
        rain_particles_[i].position = glm::vec3(
            (rand() % 1000 - 500) / 10.0f,  // -50 to 50
            (rand() % 1000 - 500) / 10.0f,  // -50 to 50
            (rand() % 500) / 10.0f + 1.0f   // 1 to 51 (above ground)
        );
        rain_particles_[i].life = (rand() % 100) / 100.0f;
        rain_particles_[i].velocity = glm::vec3(
            (rand() % 100 - 50) / 500.0f,   // Slight horizontal drift
            (rand() % 100 - 50) / 500.0f,
            -8.0f - (rand() % 40) / 10.0f   // Fall speed -8 to -12
        );
        rain_particles_[i].padding = 0.0f;
    }

    // Create VAO and VBO for rain particles
    glGenVertexArrays(1, &rain_vao_);
    glGenBuffers(1, &rain_vbo_);

    glBindVertexArray(rain_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, rain_vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RainParticle) * RAIN_PARTICLE_COUNT, rain_particles_.data(), GL_DYNAMIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RainParticle), (void*)0);
    glEnableVertexAttribArray(0);

    // Life attribute
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(RainParticle), (void*)offsetof(RainParticle, life));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    std::cout << "Rain particle system initialized with " << RAIN_PARTICLE_COUNT << " particles" << std::endl;
}

void Rasteriser::UpdateRainParticles(float delta_time, const glm::vec3& camera_pos)
{
    for (int i = 0; i < RAIN_PARTICLE_COUNT; ++i) {
        // Update position
        rain_particles_[i].position += rain_particles_[i].velocity * delta_time;

        // Update life
        rain_particles_[i].life -= delta_time * 0.5f;

        // Respawn dead or out-of-bounds particles near camera
        if (rain_particles_[i].life <= 0.0f || rain_particles_[i].position.z < -5.0f) {
            rain_particles_[i].position = camera_pos + glm::vec3(
                (rand() % 600 - 300) / 10.0f,  // -30 to 30 from camera
                (rand() % 600 - 300) / 10.0f,
                20.0f + (rand() % 200) / 10.0f  // 20 to 40 above camera
            );
            rain_particles_[i].life = 1.0f;
            rain_particles_[i].velocity = glm::vec3(
                (rand() % 100 - 50) / 500.0f,
                (rand() % 100 - 50) / 500.0f,
                -8.0f - (rand() % 40) / 10.0f
            );
        }
    }

    // Update VBO
    glBindBuffer(GL_ARRAY_BUFFER, rain_vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(RainParticle) * RAIN_PARTICLE_COUNT, rain_particles_.data());
}

//int Rasteriser::Show() {
//    while (!glfwWindowShouldClose(_window))
//    {
//        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // state setting function
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // state using function
//        glUseProgram(default_shader_program_);
//        GLint viewport[4];
//        glGetIntegerv(GL_VIEWPORT, viewport);
//        glm::mat4 P = glm::mat4(1.0f);
//        //P.set( 0, 0, float( std::min( viewport[2], viewport[3] ) ) / viewport[2] );
//        //P.set( 1, 1, float( std::min( viewport[2], viewport[3] ) ) / viewport[3] );		
//        P[0][0] = 100 * 2.0f / viewport[2];
//        P[1][1] = 100 * 2.0f / viewport[3];
//        SetMatrix4x4(default_shader_program_, glm::value_ptr(P), "MVP");
//        for (const auto& glmesh : gl_meshes_)
//        {
//            glBindVertexArray(glmesh.vao);
//            
//            glDrawElements(GL_TRIANGLES, glmesh.mesh->index_buffer_count(), GL_UNSIGNED_INT, 0);
//
//        }
//       
//
//      
//        //glDrawElements( GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0 ); // optional - render from an index buffer
//
//        glfwSwapBuffers(_window);
//        glfwPollEvents();
//    }
//    return EXIT_SUCCESS;
//}

//int Rasteriser::Show() {
//    glEnable(GL_DEPTH_TEST);
//
//    // Initialize camera (you can also do this in Init or constructor)
//    if (!camera_) {
//        camera_ = std::make_unique<Camera>();
//        camera_->SetPosition(glm::vec3(500, 10, 10));
//        camera_->SetTarget(glm::vec3(0, 0, 0));
//        camera_->SetUp(glm::vec3(0, 0, 1));
//        camera_->SetFOV(45.0);
//        // Set viewport dimensions
//        GLint viewport[4];
//        glGetIntegerv(GL_VIEWPORT, viewport);
//        camera_->SetWidth(viewport[2]);
//        camera_->SetHeight(viewport[3]);
//    }
//
//    while (!glfwWindowShouldClose(_window))
//    {
//        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        glUseProgram(default_shader_program_);
//
//        // Get matrices from camera
//        glm::mat4 V = camera_->GetViewMatrix();
//        glm::mat4 P = camera_->GetProjectionMatrix();
//        glm::vec3 camera_pos = camera_->GetPosition();
//
//        // Set lighting uniforms (in world space)
//        glm::vec3 light_ws(100.0f, 200.0f, 500.0f);
//        SetVector3(default_shader_program_, glm::value_ptr(light_ws), "light_ws");
//
//        glm::vec3 light_color(1.0f, 1.0f, 1.0f);
//        SetVector3(default_shader_program_, glm::value_ptr(light_color), "light_color");
//
//        glm::vec3 ambient(0.2f, 0.2f, 0.2f);
//        SetVector3(default_shader_program_, glm::value_ptr(ambient), "ambient_color");
//
//        glm::vec3 material_diffuse(0.8f, 0.5f, 0.2f);
//        SetVector3(default_shader_program_, glm::value_ptr(material_diffuse), "material_diffuse");
//
//        glm::vec3 material_specular(1.0f, 1.0f, 1.0f);
//        SetVector3(default_shader_program_, glm::value_ptr(material_specular), "material_specular");
//
//        float material_shininess = 32.0f;
//        SetFloat(default_shader_program_, material_shininess, "material_shininess");
//
//        // Set camera uniforms
//        SetMatrix4x4(default_shader_program_, glm::value_ptr(V), "V");
//        SetMatrix4x4(default_shader_program_, glm::value_ptr(P), "P");
//        SetVector3(default_shader_program_, glm::value_ptr(camera_pos), "camera_pos_ws");
//
//        // FIX: Create view outside the loop and use proper iteration
//        auto view = registry_.view<component::Transform, component::Mesh>();
//        for (auto entity : view) {
//            // FIX: Use get() directly on registry, not on the view
//            auto& transform = registry_.get<component::Transform>(entity);
//            auto& mesh_component = registry_.get<component::Mesh>(entity);
//
//            // Get world matrix (handles parent transformations recursively!)
//            glm::mat4 M = transform.get_world_matrix(registry_, entity);
//
//            // Normal matrix (for transforming normals)
//            glm::mat4 Mn = glm::transpose(glm::inverse(M));
//
//            // Set per-object uniforms
//            SetMatrix4x4(default_shader_program_, glm::value_ptr(M), "M");
//            SetMatrix4x4(default_shader_program_, glm::value_ptr(Mn), "Mn");
//
//            // Render all sub-meshes for this entity
//            for (const auto& glmesh : mesh_component.gl_meshes) {
//                glBindVertexArray(glmesh.vao);
//                glDrawElements(GL_TRIANGLES, glmesh.mesh->index_buffer_count(), GL_UNSIGNED_INT, 0);
//            }
//        }
//
//        glfwSwapBuffers(_window);
//        glfwPollEvents();
//    }
//
//    return EXIT_SUCCESS;
//}

void Rasteriser::CreateBindlessTexture(GLuint& texture, GLuint64& handle, const int width, const int height, const GLvoid* data, int linear)
{
    if (!data) {
        std::cout << "ERROR: Texture data is null!" << std::endl;
        handle = 0;
        return;
    }

    if (width <= 0 || height <= 0) {
        std::cout << "ERROR: Invalid texture dimensions: " << width << "x" << height << std::endl;
        handle = 0;
        return;
    }

    
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture); // bind empty texture object to the target
    // set the texture wrapping/filtering options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // copy data from the host buffer
    glTexImage2D(GL_TEXTURE_2D, 0,(linear) ? GL_RGB8 : GL_SRGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0); // unbind the newly created texture from the target
    handle = glGetTextureHandleARB(texture);
    if (handle == 0) {
        std::cout << "ERROR: Failed to create texture handle!" << std::endl;
    }
    else {
        glMakeTextureHandleResidentARB(handle);
        std::cout << "Successfully created texture handle: " << handle << std::endl;
    }// produces a handle representing the texture in a shader function
    glMakeTextureHandleResidentARB(handle);
}

int Rasteriser::Show() {
    glEnable(GL_DEPTH_TEST);
    auto entity_view = registry_.view<component::Transform, component::Mesh>();
    size_t entity_count = std::distance(entity_view.begin(), entity_view.end());
    std::cout << "Found " << entity_count << " entities" << std::endl;

    if (entity_count == 0) {
        std::cout << "ERROR: No entities to render!" << std::endl;
    }

    // Add SSBO verification
    std::cout << "Materials SSBO handle: " << materials_ssbo << std::endl;

    // Verify SSBO is bound
    GLint bound_ssbo;
    glGetIntegeri_v(GL_SHADER_STORAGE_BUFFER_BINDING, 0, &bound_ssbo);
    std::cout << "SSBO bound to binding point 0: " << bound_ssbo << std::endl;

    if (bound_ssbo != materials_ssbo) {
        std::cout << "WARNING: SSBO not properly bound! Rebinding..." << std::endl;
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, materials_ssbo);
    }

    float last_time = glfwGetTime();
    static bool printed = false;
    if (!printed) {
        auto view = registry_.view<component::Transform, component::Mesh, component::Name>();
        std::cout << "\n=== ENTITIES IN SCENE ===" << std::endl;
        for (auto [entity, transform, mesh, name] : view.each()) {
            std::cout << "Entity: " << name.value << std::endl;
            std::cout << "  Position: (" << transform.translation.x << ", "
                << transform.translation.y << ", "
                << transform.translation.z << ")" << std::endl;
            std::cout << "  Scale: (" << transform.scale.x << ", "
                << transform.scale.y << ", "
                << transform.scale.z << ")" << std::endl;
            std::cout << "  Mesh count: " << mesh.gl_meshes.size() << std::endl;
        }

        glm::vec3 cam_pos = camera_->GetPosition();
        glm::vec3 cam_target = camera_->GetTarget();
        std::cout << "\nCamera Position: (" << cam_pos.x << ", " << cam_pos.y << ", " << cam_pos.z << ")" << std::endl;
        std::cout << "Camera Target: (" << cam_target.x << ", " << cam_target.y << ", " << cam_target.z << ")" << std::endl;
        std::cout << "Camera FOV: " << camera_->GetFOV() << std::endl;
        std::cout << "Camera Near: " << camera_->GetNear() << std::endl;
        std::cout << "Camera Far: " << camera_->GetFar() << std::endl;
        std::cout << "=========================\n" << std::endl;
        printed = true;
    }
    // Light position and shadow mapping setup
    glm::vec3 light_ws(30.0f, -30.0f, 60.0f);  // High above and to the side
    glm::vec3 light_target(0.0f, 0.0f, 0.0f);  // Light looks at scene center
    glm::vec3 light_up(0.0f, 0.0f, 1.0f);

    // Light's view matrix (looking from light toward scene)
    glm::mat4 V_light = glm::lookAt(light_ws, light_target, light_up);

    // Orthographic projection for directional light shadow
    float shadow_size = 40.0f;  // Size of the shadow frustum
    glm::mat4 P_light = glm::ortho(-shadow_size, shadow_size, -shadow_size, shadow_size, 1.0f, 150.0f);

    // Combined light-space projection matrix (P_light * V_light)
    glm::mat4 light_space_matrix = P_light * V_light;

    // Bind shadow map texture to texture unit 3 before entering the loop
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, tex_shadow_map_);
    SetSampler(default_shader_program_, 3, "shadow_map");

    while (!glfwWindowShouldClose(_window))
    {
        static int frame = 0;
        if (frame++ % 60 == 0) {  // Print every 60 frames
            glm::vec3 pos = camera_->GetPosition();
            std::cout << "Camera: (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
        }
        // Calculate delta time
        float current_time = glfwGetTime();
        float delta_time = current_time - last_time;
        last_time = current_time;

        // Clamp delta time to reasonable values
        if (delta_time <= 0.0f || delta_time > 0.1f) {
            delta_time = 0.016f;  // Default to ~60 FPS
        }
        // Update physics
        PhysicsManager::Instance().Update(delta_time);

        // Update player (movement, camera, etc.)
        if (player_) {
            player_->Update(delta_time);
        }

        // ===== SHADOW PASS: Render scene from light's perspective =====
        if (shadow_program_ != 0) {
            glUseProgram(shadow_program_);
            glViewport(0, 0, shadow_width_, shadow_height_);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo_shadow_map_);
            glClear(GL_DEPTH_BUFFER_BIT);

            // Disable culling for shadow pass to render all geometry
            glDisable(GL_CULL_FACE);

            // Use polygon offset to help with shadow acne
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(2.0f, 4.0f);

            // Render all opaque objects to shadow map
            auto shadow_view = registry_.view<component::Transform, component::Mesh>(entt::exclude<component::Grass>);
            for (auto [entity, transform, mesh_component] : shadow_view.each()) {
                glm::mat4 M = transform.get_world_matrix(registry_, entity);
                glm::mat4 mlp = light_space_matrix * M;  // Model-Light-Projection

                SetMatrix4x4(shadow_program_, glm::value_ptr(mlp), "mlp");

                for (const auto& glmesh : mesh_component.gl_meshes) {
                    glBindVertexArray(glmesh.vao);
                    glDrawElements(GL_TRIANGLES, glmesh.mesh->index_buffer_count(), GL_UNSIGNED_INT, 0);
                }
            }

            // Restore state
            glDisable(GL_POLYGON_OFFSET_FILL);
            glEnable(GL_CULL_FACE);

            // Reset to default framebuffer and viewport
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, width_, height_);
        }

        // ===== MAIN PASS: Render scene with shadows =====
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Get matrices from camera (now controlled by player)
        glm::mat4 V = camera_->GetViewMatrix();
        glm::mat4 P = camera_->GetProjectionMatrix();
        glm::vec3 camera_pos = camera_->GetPosition();

        // ===== PASS 0: Render skybox (environment background) =====
        if (skybox_shader_program_ != 0 && skybox_texture_handle_ != 0) {
            // Disable depth writing (skybox is always at infinity)
            glDepthMask(GL_FALSE);
            glDisable(GL_CULL_FACE);

            glUseProgram(skybox_shader_program_);

            // Calculate inverse VP matrix for ray direction computation
            glm::mat4 VP = P * V;
            glm::mat4 inv_VP = glm::inverse(VP);
            SetMatrix4x4(skybox_shader_program_, glm::value_ptr(inv_VP), "inv_VP");

            // Set skybox texture handle
            GLint loc = glGetUniformLocation(skybox_shader_program_, "skybox_texture");
            if (loc != -1) {
                glUniform2ui(loc,
                    static_cast<GLuint>(skybox_texture_handle_ & 0xFFFFFFFF),
                    static_cast<GLuint>(skybox_texture_handle_ >> 32));
            }

            // Draw fullscreen triangle
            glBindVertexArray(skybox_vao_);
            glDrawArrays(GL_TRIANGLES, 0, 3);

            // Restore state
            glDepthMask(GL_TRUE);
            glEnable(GL_CULL_FACE);
        }

        glUseProgram(default_shader_program_);

        // Set lighting uniforms - sun-like directional light from above
        glm::vec3 light_ws(30.0f, -30.0f, 60.0f);  // High above and to the side
        SetVector3(default_shader_program_, glm::value_ptr(light_ws), "light_ws");

        glm::vec3 light_color(1.8f, 1.8f, 1.7f);  // Bright warm sunlight
        SetVector3(default_shader_program_, glm::value_ptr(light_color), "light_color");

        glm::vec3 ambient(0.25f, 0.25f, 0.3f);  // Reduced ambient for visible shadows
        SetVector3(default_shader_program_, glm::value_ptr(ambient), "ambient_color");

        // Set camera uniforms
        SetMatrix4x4(default_shader_program_, glm::value_ptr(V), "V");
        SetMatrix4x4(default_shader_program_, glm::value_ptr(P), "P");
        SetVector3(default_shader_program_, glm::value_ptr(camera_pos), "camera_pos_ws");

        // Set light space matrix for shadow mapping
        SetMatrix4x4(default_shader_program_, glm::value_ptr(light_space_matrix), "light_space_matrix");

        // Bind shadow map
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, tex_shadow_map_);

        // ===== Render opaque objects (non-grass) =====
        auto opaque_view = registry_.view<component::Transform, component::Mesh>(entt::exclude<component::Grass>);
        for (auto [entity, transform, mesh_component] : opaque_view.each()) {
            glm::mat4 M = transform.get_world_matrix(registry_, entity);
            glm::mat3 Mn = glm::transpose(glm::inverse(glm::mat3(M)));

            SetMatrix4x4(default_shader_program_, glm::value_ptr(M), "M");
            SetMatrix3x3(default_shader_program_, glm::value_ptr(Mn), "Mn");

            for (const auto& glmesh : mesh_component.gl_meshes) {
                glBindVertexArray(glmesh.vao);
                glDrawElements(GL_TRIANGLES, glmesh.mesh->index_buffer_count(), GL_UNSIGNED_INT, 0);
            }
        }

        // ===== Render transparent objects (grass) with blending =====
        if (grass_shader_program_ != 0) {
            // Enable alpha blending
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Disable face culling for grass (visible from both sides)
            glDisable(GL_CULL_FACE);

            // Use grass shader
            glUseProgram(grass_shader_program_);

            // Set uniforms for grass shader
            SetMatrix4x4(grass_shader_program_, glm::value_ptr(V), "V");
            SetMatrix4x4(grass_shader_program_, glm::value_ptr(P), "P");
            SetVector3(grass_shader_program_, glm::value_ptr(light_ws), "light_ws");
            SetVector3(grass_shader_program_, glm::value_ptr(light_color), "light_color");
            SetVector3(grass_shader_program_, glm::value_ptr(ambient), "ambient_color");
            SetVector3(grass_shader_program_, glm::value_ptr(camera_pos), "camera_pos_ws");

            // Set time uniform for wind animation
            SetFloat(grass_shader_program_, current_time, "time");

            // Set shadow uniforms for grass
            SetMatrix4x4(grass_shader_program_, glm::value_ptr(light_space_matrix), "light_space_matrix");
            SetSampler(grass_shader_program_, 3, "shadow_map");

            // Render grass entities
            auto grass_view = registry_.view<component::Transform, component::Mesh, component::Grass>();

            for (auto [entity, transform, mesh_component] : grass_view.each()) {
                glm::mat4 M = transform.get_world_matrix(registry_, entity);
                glm::mat3 Mn = glm::transpose(glm::inverse(glm::mat3(M)));

                SetMatrix4x4(grass_shader_program_, glm::value_ptr(M), "M");
                SetMatrix3x3(grass_shader_program_, glm::value_ptr(Mn), "Mn");

                for (const auto& glmesh : mesh_component.gl_meshes) {
                    glBindVertexArray(glmesh.vao);
                    glDrawElements(GL_TRIANGLES, glmesh.mesh->index_buffer_count(), GL_UNSIGNED_INT, 0);
                }
            }

            // Restore state
            glDisable(GL_BLEND);
            glEnable(GL_CULL_FACE);
        }

        // ===== Render rain particles =====
        if (rain_shader_program_ != 0 && rain_vao_ != 0) {
            // Update rain particles
            UpdateRainParticles(delta_time, camera_pos);

            // Enable blending for transparent rain
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Disable depth writing but keep depth testing
            glDepthMask(GL_FALSE);

            // Enable point sprites
            glEnable(GL_PROGRAM_POINT_SIZE);

            glUseProgram(rain_shader_program_);

            // Set uniforms
            glm::mat4 VP = P * V;
            SetMatrix4x4(rain_shader_program_, glm::value_ptr(VP), "VP");
            SetVector3(rain_shader_program_, glm::value_ptr(camera_pos), "camera_pos");
            SetFloat(rain_shader_program_, current_time, "time");

            // Draw rain particles as points
            glBindVertexArray(rain_vao_);
            glDrawArrays(GL_POINTS, 0, RAIN_PARTICLE_COUNT);

            // Restore state
            glDisable(GL_PROGRAM_POINT_SIZE);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }

        glfwSwapBuffers(_window);
        glfwPollEvents();
    }


    return EXIT_SUCCESS;
}

// Add these functions to Rasteriser.cpp

void Rasteriser::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Rasteriser* rast = static_cast<Rasteriser*>(glfwGetWindowUserPointer(window));

    if (rast && rast->player_) {
        rast->player_->ProcessKeyboard(key, action);
    }
    else if (rast) {
        // Use orbit camera controls when player is not active
        rast->ProcessCameraInput(key, action);
    }

    // ESC to close window
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void Rasteriser::ProcessCameraInput(int key, int action) {
    if (action == GLFW_RELEASE) return;  // Only process press and repeat

    const float angle_step = 0.05f;    // Rotation speed (radians)
    const float distance_step = 1.0f;  // Zoom speed
    const float pitch_step = 0.03f;    // Vertical rotation speed

    // Calculate forward and right vectors based on current orbit angle
    glm::vec3 forward(cos(orbit_angle_), sin(orbit_angle_), 0.0f);
    glm::vec3 right(sin(orbit_angle_), -cos(orbit_angle_), 0.0f);

    switch (key) {
        // Rotation controls (Arrow keys)
        case GLFW_KEY_RIGHT:
            orbit_angle_ -= angle_step;  // Rotate right (clockwise)
            break;
        case GLFW_KEY_LEFT:
            orbit_angle_ += angle_step;  // Rotate left (counter-clockwise)
            break;
        case GLFW_KEY_UP:
            orbit_pitch_ += pitch_step;  // Look up
            orbit_pitch_ = std::min(orbit_pitch_, 1.4f);
            break;
        case GLFW_KEY_DOWN:
            orbit_pitch_ -= pitch_step;  // Look down
            orbit_pitch_ = std::max(orbit_pitch_, -0.2f);
            break;

        // Movement controls (WASD) - move the camera target point
        case GLFW_KEY_W:
            orbit_target_ += forward * move_speed_;  // Move forward
            break;
        case GLFW_KEY_S:
            orbit_target_ -= forward * move_speed_;  // Move backward
            break;
        case GLFW_KEY_A:
            orbit_target_ -= right * move_speed_;    // Move left
            break;
        case GLFW_KEY_D:
            orbit_target_ += right * move_speed_;    // Move right
            break;

        // Zoom controls
        case GLFW_KEY_Q:
            orbit_distance_ -= distance_step;  // Zoom in
            orbit_distance_ = std::max(orbit_distance_, 1.0f);  // Can get very close
            break;
        case GLFW_KEY_E:
            orbit_distance_ += distance_step;  // Zoom out
            orbit_distance_ = std::min(orbit_distance_, 100.0f);
            break;

        // Vertical movement
        case GLFW_KEY_R:
            orbit_target_.z += move_speed_;  // Move up
            break;
        case GLFW_KEY_F:
            orbit_target_.z -= move_speed_;  // Move down
            break;
    }

    UpdateOrbitCamera();
}

void Rasteriser::UpdateOrbitCamera() {
    // Calculate camera position on a sphere around the target
    float x = orbit_distance_ * cos(orbit_pitch_) * cos(orbit_angle_);
    float y = orbit_distance_ * cos(orbit_pitch_) * sin(orbit_angle_);
    float z = orbit_distance_ * sin(orbit_pitch_);

    glm::vec3 new_pos = orbit_target_ + glm::vec3(x, y, z);

    camera_->SetPosition(new_pos);
    camera_->SetTarget(orbit_target_);
}

void Rasteriser::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    Rasteriser* rast = static_cast<Rasteriser*>(glfwGetWindowUserPointer(window));

    if (rast && rast->player_) {
        rast->player_->ProcessMouseMovement(xpos, ypos);
    }
}