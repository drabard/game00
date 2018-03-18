#pragma once

#include "GL/gl3w.h"
#include "GL/gl.h"

typedef GLuint gpu_program;
typedef GLuint gpu_shader;

#define GPU_VERTEX_SHADER GL_VERTEX_SHADER
#define GPU_FRAGMENT_SHADER GL_FRAGMENT_SHADER
#define GPU_MAX_TEXTURE_UNITS GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS

enum gpu_status { GPU_OK = 0,
                  GPU_FAILURE };

enum gpu_vertex_flags {
    GPU_POS = (1 << 0), // positions; size: 4 * sizeof(float)
    GPU_NORM = (1 << 1), // normals; size: 3 * sizeof(float)
    GPU_TEXCOORD = (1 << 2) // texture coords; size: 2 * sizeof(float)
};
extern const uint32_t gpu_max_vert_bytes;

typedef void (*gpu_log_fptr)(const char*, ...);
void gpu_set_log(gpu_log_fptr l);

enum gpu_status gpu_init();

typedef uint8_t gpu_vtx_flags_t;

gpu_vtx_flags_t gpu_pack_verts(void* outbuf, float* positions, float* normals,
                               float* texcoords, uint32_t nverts);

// ---- Shaders ----

enum gpu_status gpu_link_program(gpu_program* dst, gpu_shader vertex_shader,
                                 gpu_shader fragment_shader);

enum gpu_status gpu_compile_shader(gpu_shader* dst, GLenum shader_type,
                                   const char* source);

enum gpu_status gpu_activate_program(gpu_program p);

// ---- Vertex buffer ----

struct gpu_vertex_buffer {
    GLuint vao;
    GLuint vertex_buf;
    GLuint elem_buf;

    uint32_t nindices;
};

enum gpu_status gpu_vertex_buffer_create(struct gpu_vertex_buffer* vertex_buffer, const void* vertices, gpu_vtx_flags_t vert_flags, const uint32_t* indices, uint32_t nverts, uint32_t nindices);

void gpu_vertex_buffer_destroy(struct gpu_vertex_buffer* vertex_buffer);

void gpu_vertex_buffer_draw(const struct gpu_vertex_buffer* tgt);

// ---- Texture ----

struct gpu_texture {
    GLuint id;
};

enum gpu_status gpu_texture_create_RGBA(struct gpu_texture* dst, const uint8_t* data, uint32_t width, uint32_t height);

enum gpu_status gpu_texture_create_R(struct gpu_texture* dst, const uint8_t* data, uint32_t width, uint32_t height);

enum gpu_status gpu_texture_bind(struct gpu_texture* tex, const gpu_program* programs,
                             uint32_t nprograms, uint32_t tex_unit,
                             const char* sampler_name);

void gpu_texture_destroy(struct gpu_texture* tgt);

// ---- Uniform ----

typedef GLint gpu_uniform;

enum gpu_status gpu_get_uniform(gpu_program program, const char* name,
                                gpu_uniform* uniform);

enum gpu_status gpu_set_uniform_i(gpu_uniform uniform, int32_t value);
enum gpu_status gpu_set_uniform_2f(gpu_uniform uniform, const float* values);
enum gpu_status gpu_set_uniform_3f(gpu_uniform uniform, const float* values);
enum gpu_status gpu_set_uniform_m4(gpu_uniform uniform, const float* values);

// ---- Uniform buffer ----

struct gpu_uniform_buf {
    const char* name;
    GLuint buf; // OpenGL buffer ID
    GLuint bind_loc; // Binding location
};

enum gpu_status gpu_uniform_buf_create(struct gpu_uniform_buf* ub,
                                       uint32_t bind_location, const char* name,
                                       void* data, uint32_t nbytes);

void gpu_uniform_buf_destroy(struct gpu_uniform_buf* ub);

enum gpu_status gpu_uniform_buf_update(struct gpu_uniform_buf* ub, void* data,
                                       uint32_t nbytes);

enum gpu_status gpu_uniform_buf_bind(struct gpu_uniform_buf* ub,
                                     const gpu_program* programs, uint32_t nprograms);
