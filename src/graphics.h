#pragma once

#include "gpu.h"
#include "math.h"

// todo remove this dependency
// resources should be managed by the user, and only already uploaded gpu
// stuff should be used here.
#include "resources.h"

typedef void (*gfx_log_fptr)(const char*, ...);
void gfx_set_log(gfx_log_fptr l);

typedef void* (*gfx_malloc_fptr)(size_t);
typedef void (*gfx_free_fptr)(void*);
typedef void* (*gfx_realloc_fptr)(void*, size_t);
void gfx_set_mem(gfx_malloc_fptr m, gfx_free_fptr f, gfx_realloc_fptr r);

enum gfx_status { GFX_OK = 0,
                  GFX_FAILURE };

enum gfx_status gfx_init(uint32_t screen_size[2]);

// ---- Shaders ---------------------------

enum gfx_shader_type {
    GFX_VERTEX_SHADER = GPU_VERTEX_SHADER,
    GFX_FRAGMENT_SHADER = GPU_FRAGMENT_SHADER
};

struct gfx_shader_def {
    const char* name;
    const char* source;
    enum gfx_shader_type type;
};

struct gfx_program_def {
    const char* name;
    const char* vertex_shader_name;
    const char* fragment_shader_name;
};

struct gfx_shader {
    const char* name;
    enum gfx_shader_type type;
    gpu_shader shader;
};

struct gfx_program {
    const char* name;
    gpu_program program;
};

struct gfx_program_storage {
    struct gfx_shader* shaders;
    uint32_t nshaders;

    struct gfx_program* programs;
    uint32_t nprograms;
};

enum gfx_status gfx_compile_shaders(struct gfx_program_storage* storage,
                                 const struct gfx_shader_def* defs,
                                 uint32_t ndefs);

enum gfx_status gfx_compile_programs(struct gfx_program_storage* storage,
                                     const struct gfx_program_def* defs,
                                     uint32_t ndefs);

enum gfx_status gfx_get_program(struct gfx_program_storage* storage, const char* program_name,
                                struct gfx_program** program);

enum gfx_status gfx_activate_program(struct gfx_program* program);

// ---- Perspective and view ----

enum gfx_status gfx_update_view(const m4* view, struct gfx_program* program);
enum gfx_status gfx_update_projection(const m4* projection, struct gfx_program* program);

// ---- Mesh ----

struct gfx_mesh
{
  struct gpu_vertex_buffer vertex_buffer;
  struct gpu_texture* textures; // owned
  uint32_t ntextures;
};

enum gfx_status gfx_mesh_create(struct gfx_mesh* mesh, const struct rsrc_mesh* resource, const struct rsrc_texture* tex_rsrcs, uint32_t ntextures);

void gfx_mesh_destroy(struct gfx_mesh* mesh);

enum gfx_status gfx_mesh_draw(struct gfx_mesh* mesh, const struct gfx_program* active_program, const m4* transforms, uint32_t ntransforms);

// ---- Font ----

struct gfx_font
{
  struct gpu_texture texture;
  const struct rsrc_font* resource; // not owned
};

enum gfx_status gfx_font_create(struct gfx_font* fnt, const struct rsrc_font* rsrc);

void gfx_font_destroy(struct gfx_font* fnt);

// ---- Text ----

struct gfx_text
{
  const char* text_ansi; // not owned
  struct gfx_font* font; // not owned
  struct gpu_vertex_buffer quads;
};

enum gfx_status gfx_text_create(struct gfx_text* txt, const char* text_ansi, struct gfx_font* font );

void gfx_text_destroy(struct gfx_text* txt);

enum gfx_status gfx_text_draw(struct gfx_text* txt, const struct gfx_program* active_program, float screen_x, float screen_y);