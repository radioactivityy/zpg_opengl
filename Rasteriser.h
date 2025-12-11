#pragma once
#include <string>
#include "glutils.h"
#include "glmaterial.h"
#include "component.h"
#include "Camera.h"
#include "player.h"
#include "collider.h"
#include <vector>


class Rasteriser
{
public:
    Rasteriser();
    ~Rasteriser();
    void AddCollisionFromOBJ(const std::string& obj_path, const glm::vec3& position = glm::vec3(0.0f));

    int InitOpenGLContext();
    int LoadMesh(const std::string& s, std::vector<GLMesh> & gl_meshes);
    void CreateBindlessTexture(GLuint& texture, GLuint64& handle, const int width, const int height, const GLvoid* data, int linear);

    entt::registry& GetRegistry() { return registry_; }  // Add this
    // In Rasteriser.h
    entt::entity CreateEntity(const std::string& mesh_file, const std::string& name, entt::entity parent = entt::null);
    int Show();
    int LoadProgram(const std::string& vs_file_name, const std::string& fs_file_name);
    int LoadGrassProgram(const std::string& vs_file_name, const std::string& fs_file_name);
    int LoadShadowProgram(const std::string& vs_file_name, const std::string& fs_file_name);
    void InitShadowDepthbuffer();
private:
    std::vector<std::shared_ptr<TriangularMesh>> meshes_;
    entt::registry registry_;
    std::unique_ptr<MeshLoader> _mesh_loader;
    int width_{ 800 };
    int height_{ 800 };
    GLFWwindow* _window;
    std::unique_ptr<Camera> camera_;
    GLuint default_shader_program_{ 0 };
    GLuint grass_shader_program_{ 0 };  // Grass shader with wind animation
    GLuint materials_ssbo{ 0 };
    std::vector<GLMaterial> materials_;
    std::unique_ptr<Player> player_;

    // Shadow mapping
    int shadow_width_{ 2048 };  // shadow map resolution
    int shadow_height_{ 2048 };
    GLuint fbo_shadow_map_{ 0 };  // shadow mapping FBO
    GLuint tex_shadow_map_{ 0 };  // shadow map texture
    GLuint shadow_program_{ 0 };  // shadow mapping shaders

    // Camera orbit controls
    float orbit_angle_{ 0.0f };      // Horizontal angle around target (radians)
    float orbit_pitch_{ 0.3f };      // Vertical angle (radians)
    float orbit_distance_{ 25.0f };  // Distance from target
    glm::vec3 orbit_target_{ 0.0f, 0.0f, 2.0f };  // Point camera orbits around
    float move_speed_{ 0.5f };       // Speed for moving the target point

    void UpdateOrbitCamera();
    void ProcessCameraInput(int key, int action);

    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
};
