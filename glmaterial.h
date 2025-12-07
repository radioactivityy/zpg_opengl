// Match the GPU layout exactly with proper alignment
#pragma pack(push, 1)  // 1 byte alignment
#include "glutils.h"
struct GLMaterial {
    Color3f diffuse;           // 12 bytes
    GLbyte pad0[4];               // 4 bytes padding = 16 bytes total
    GLuint64 tex_diffuse_handle; // 8 bytes (bindless texture handle)
    GLbyte pad1[8];               // 8 bytes padding = 16 bytes total

    Color3f rma;                // 12 bytes (roughness, metalness, ao)
    GLbyte pad2[4];               // 4 bytes padding = 16 bytes total
    GLuint64 tex_rma_handle;      // 8 bytes
    GLbyte pad3[8];               // 8 bytes padding = 16 bytes total

    Color3f normal;             // 12 bytes
    GLbyte pad4[4];               // 4 bytes padding = 16 bytes total
    GLuint64 tex_normal_handle;   // 8 bytes
    GLbyte pad5[8];               // 8 bytes padding = 16 bytes total

    // Total size: 96 bytes (6 * 16)
};
#pragma pack(pop)