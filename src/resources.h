#pragma once

#include <stdint.h>
#include <stddef.h>

// todo: endianness (when needed)

typedef uint64_t rsrc_id;

enum rsrc_status {
    RSRC_OK = 0,
    RSRC_FAILURE
};

typedef void (*rsrc_log_fptr)(const char*, ...);
void rsrc_set_log(rsrc_log_fptr l);

typedef void* (*rsrc_malloc_fptr)(size_t);
typedef void (*rsrc_free_fptr)(void*);
typedef void* (*rsrc_realloc_fptr)(void*, size_t);
void rsrc_set_mem(rsrc_malloc_fptr m, rsrc_free_fptr f, rsrc_realloc_fptr r);

enum rsrc_status rsrc_init();

// ---- Mesh -------------------------------------------------------------------

static const uint8_t rsrc_mesh_version = 0;

struct rsrc_mesh {
    float* positions; // size: nverts*3*sizeof(float)
    float* normals; // size: nverts*3*sizeof(float)
    float* texcoords; // size: nverts*3*sizeof(float)
    uint32_t* indices; // size: nindices*sizeof(uint32_t)

    uint32_t nverts;
    uint32_t nindices;
};

enum rsrc_status rsrc_mesh_load(struct rsrc_mesh* res, const uint8_t* buffer,
                                uint32_t buf_size);
void rsrc_mesh_unload(struct rsrc_mesh* res);
enum rsrc_status rsrc_mesh_save(const struct rsrc_mesh* res, uint8_t* buffer,
                                uint32_t buf_size);
uint64_t rsrc_mesh_buf_size(const struct rsrc_mesh* res);

// ---- Texture -----------------------------------------------------------------

static const uint8_t rsrc_texture_version = 0;

struct rsrc_texture {
    uint8_t* data;

    int32_t width;
    int32_t height;
    int32_t ncomps;
};

enum rsrc_status rsrc_texture_load(struct rsrc_texture* res, const uint8_t* buffer,
                                   uint32_t buf_size);
void rsrc_texture_unload(struct rsrc_texture* res);

// ---- Font -------------------------------------------------------------------

static const uint8_t rsrc_font_version = 1;

struct rsrc_font_glyph {
    uint64_t charcode;
    float advance_x;
    float bearing_x;
    float bearing_y;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
};

struct rsrc_font {
    uint16_t nglyphs;
    uint16_t bmp_height;
    uint16_t bmp_width;
    float line_spacing;
    uint8_t* bitmap;
    struct rsrc_font_glyph* glyphs;
};

enum rsrc_status rsrc_font_load(struct rsrc_font* res, const uint8_t* buffer,
                                uint32_t buf_size);
void rsrc_font_unload(struct rsrc_font* res);

enum rsrc_status rsrc_font_save(const struct rsrc_font* res, uint8_t* buffer,
                                uint32_t buf_size);
uint64_t rsrc_font_buf_size(const struct rsrc_font* res);
