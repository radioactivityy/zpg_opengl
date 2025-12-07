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
private:
    std::vector<std::shared_ptr<TriangularMesh>> meshes_;
    entt::registry registry_;
    std::unique_ptr<MeshLoader> _mesh_loader;
    int width_{ 480 };
    int height_{ 480 };
    GLFWwindow* _window;
    std::unique_ptr<Camera> camera_;
    GLuint default_shader_program_{ 0 };
    GLuint materials_ssbo{ 0 };
    std::vector<GLMaterial> materials_;
    std::unique_ptr<Player> player_;

    
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
};
