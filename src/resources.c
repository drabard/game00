#include "resources.h"

#include <stdlib.h>
#include <stdio.h>

static rsrc_log_fptr text_log = NULL;

static rsrc_malloc_fptr rsrc_malloc = NULL;
static rsrc_free_fptr rsrc_free = NULL;
static rsrc_realloc_fptr rsrc_realloc = NULL;

#define STB_IMAGE_IMPLEMENTATION
#define STBI_MALLOC(sz) rsrc_malloc(sz)
#define STBI_FREE(p) rsrc_free(p)
#define STBI_REALLOC(p, sz) rsrc_realloc(p, sz)
#include "stb_image.h"

// ---- Serialization ----------------------------------------------------------

static void rsrc_memcpy(void* dst, const void* src, size_t nbytes)
{
    uint8_t* d = dst;
    const uint8_t* s = src;
    for (; nbytes; --nbytes)
        *d++ = *s++;
}

static enum rsrc_status write_bytes(const void* src, uint32_t nbytes, uint8_t** buffer,
                                    uint32_t* rem_size)
{
    if (*rem_size < nbytes)
        return RSRC_FAILURE;

    rsrc_memcpy(*buffer, src, nbytes);

    *rem_size -= nbytes;
    *buffer += nbytes;

    return RSRC_OK;
}

static enum rsrc_status read_bytes(void* dst, uint32_t nbytes, const uint8_t** buffer,
                                   uint32_t* rem_size)
{
    if (*rem_size < nbytes)
        return RSRC_FAILURE;

    rsrc_memcpy(dst, *buffer, nbytes);

    *rem_size -= nbytes;
    *buffer += nbytes;

    return RSRC_OK;
}

// ---- Initialization ---------------------------------------------------------

void rsrc_set_log(rsrc_log_fptr l)
{
    text_log = l;
}

void rsrc_set_mem(rsrc_malloc_fptr m, rsrc_free_fptr f, rsrc_realloc_fptr r)
{
    rsrc_malloc = m;
    rsrc_free = f;
    rsrc_realloc = r;
}

enum rsrc_status rsrc_init()
{
    stbi_set_flip_vertically_on_load(1);
    return RSRC_OK;
}

// ---- Mesh -------------------------------------------------------------------

enum vertex_flags
{
    VTX_TEXCOORDS = 1,
    VTX_NORMALS = 1 << 1,
};

enum rsrc_status rsrc_mesh_load(struct rsrc_mesh* res, const uint8_t* buf,
                                uint32_t bufnb)
{
    uint8_t version;

    uint8_t* buffer = 0;

    if (read_bytes(&version, sizeof(version), &buf, &bufnb) != RSRC_OK)
        goto error;

    if (version != rsrc_mesh_version) {
        text_log("ERROR: Mesh version mismatch (compiled: %d, loading: %d).\n",
                 rsrc_font_version, version);
        goto error;
    }

    uint32_t nverts;
    uint32_t nindices;
    if(read_bytes(&nverts, sizeof(nverts), &buf, &bufnb) != RSRC_OK)
        goto error;
    if(read_bytes(&nindices, sizeof(nindices), &buf, &bufnb) != RSRC_OK)
        goto error;

    uint8_t flags;
    if(read_bytes(&flags, sizeof(flags), &buf, &bufnb) != RSRC_OK)
        goto error;

    uint32_t positions_bytes = nverts * sizeof(float) * 3;
    uint32_t index_bytes = nindices * sizeof(uint32_t);

    uint32_t texcoords_bytes = 0;
    if( ( flags & VTX_TEXCOORDS ) != 0 )
    {
        texcoords_bytes = nverts * sizeof(float) * 3;
    }

    uint32_t normals_bytes = 0;
    if( ( flags & VTX_NORMALS ) != 0 )
    {
        normals_bytes = nverts * sizeof(float) * 3;
    }

    buffer = rsrc_malloc(positions_bytes + texcoords_bytes + normals_bytes + index_bytes);
    if( !buffer )
    {
        text_log("ERROR: Out of memory.\n");
        goto error;
    }

    float* positions = (float*) buffer;
    if(read_bytes(positions, positions_bytes, &buf, &bufnb) != RSRC_OK) goto error;

    float* texcoords = 0;
    if( texcoords_bytes != 0 )
    {
        texcoords = (float*) (buffer + positions_bytes);
        if(read_bytes(texcoords, texcoords_bytes, &buf, &bufnb) != RSRC_OK) goto error;
    }

    float* normals = 0;
    if( normals_bytes != 0 )
    {
        normals = (float*) (buffer + positions_bytes + texcoords_bytes);
        if(read_bytes(normals, normals_bytes, &buf, &bufnb) != RSRC_OK) goto error;
    }

    uint32_t* indices = (uint32_t*) (buffer + positions_bytes + texcoords_bytes + normals_bytes);
    if(read_bytes(indices, index_bytes, &buf, &bufnb) != RSRC_OK) goto error;

    res->nverts = nverts;
    res->nindices = nindices;
    res->positions = positions;
    res->texcoords = texcoords;
    res->normals = normals;
    res->indices = indices;

    return RSRC_OK;

error:
    rsrc_free(buffer);
    return RSRC_FAILURE;
}

void rsrc_mesh_unload(struct rsrc_mesh* res)
{
    rsrc_free(res->positions);
    res->nverts = 0;
    res->nindices = 0;
}

enum rsrc_status rsrc_mesh_save(const struct rsrc_mesh* res, uint8_t* buf,
                                uint32_t bufnb)
{
    if (bufnb < rsrc_mesh_buf_size(res))
        return RSRC_FAILURE;

    if (write_bytes(&rsrc_mesh_version, sizeof(rsrc_mesh_version), &buf,
                    &bufnb) != RSRC_OK) goto error;
    if (write_bytes(&res->nverts, sizeof(res->nverts), &buf, &bufnb) != RSRC_OK)
        goto error;
    if (write_bytes(&res->nindices, sizeof(res->nindices), &buf, &bufnb) != RSRC_OK)
        goto error;

    uint8_t flags = 0;
    if(res->texcoords != 0) flags |= VTX_TEXCOORDS;
    if(res->normals != 0) flags |= VTX_NORMALS;
    if (write_bytes(&flags, sizeof(flags), &buf, &bufnb) != RSRC_OK) 
        goto error;

    if (write_bytes(res->positions, res->nverts * 3 * sizeof(float), &buf, &bufnb) != RSRC_OK) 
        goto error;

    if(res->texcoords != 0)
    {
        if (write_bytes(res->texcoords, res->nverts * 3 * sizeof(float), &buf, &bufnb) != RSRC_OK) 
            goto error;
    }

    if(res->normals != 0)
    {
        if (write_bytes(res->normals, res->nverts * 3 * sizeof(float), &buf, &bufnb) != RSRC_OK) 
            goto error;
    }

    if (write_bytes(res->indices, res->nindices * sizeof(uint32_t), &buf, &bufnb) != RSRC_OK) 
        goto error;

    return RSRC_OK;

error:
    return RSRC_FAILURE;
}

uint64_t rsrc_mesh_buf_size(const struct rsrc_mesh* res)
{
    uint64_t metadata_size = sizeof(rsrc_mesh_version) + sizeof(res->nindices) + sizeof(res->nverts) + sizeof(uint8_t);
    uint64_t positions_bytes = sizeof(float) * 3 * res->nverts;
    uint64_t texcoords_bytes = res->texcoords == 0 ? 0 : sizeof(float) * 3 * res->nverts;
    uint64_t normals_bytes = res->normals == 0 ? 0 : sizeof(float) * 3 * res->nverts;
    uint64_t indices_bytes = sizeof(uint32_t) * res->nindices;
    return metadata_size + positions_bytes + texcoords_bytes + normals_bytes + indices_bytes;
}

// ---- Texture-----------------------------------------------------------------

enum rsrc_status rsrc_texture_load(struct rsrc_texture* res, const uint8_t* buffer,
                                   uint32_t buf_size)
{
    int width, height;
    int n = 4;
    uint8_t* data = stbi_load_from_memory(buffer, buf_size, &width, &height, &n, 0);
    if (!data)
        return RSRC_FAILURE;

    res->width = width;
    res->height = height;
    res->ncomps = n;
    res->data = data;

    return RSRC_OK;
}

void rsrc_texture_unload(struct rsrc_texture* res)
{
    stbi_image_free(res->data);
    *res = (struct rsrc_texture){};
}

// ---- Font -------------------------------------------------------------------

enum rsrc_status rsrc_font_load(struct rsrc_font* res, const uint8_t* buffer,
                                uint32_t buf_size)
{
    struct rsrc_font_glyph* glyphs = 0;
    uint8_t* bitmap = 0;

    const uint8_t* b = buffer;

    uint8_t version;

    if (read_bytes(&version, sizeof(version), &b, &buf_size) != RSRC_OK)
        goto error;
    if (version != rsrc_font_version) {
        text_log("ERROR: Font version mismatch (compiled: %d, loading: %d).\n",
                 rsrc_font_version, version);
        goto error;
    }

    uint16_t nglyphs, bmp_height, bmp_width;
    float line_spacing;
    if (read_bytes(&nglyphs, sizeof(nglyphs), &b, &buf_size) != RSRC_OK)
        goto error;
    if (read_bytes(&bmp_height, sizeof(bmp_height), &b, &buf_size) != RSRC_OK)
        goto error;
    if (read_bytes(&bmp_width, sizeof(bmp_width), &b, &buf_size) != RSRC_OK)
        goto error;
    if (read_bytes(&line_spacing, sizeof(line_spacing), &b, &buf_size) != RSRC_OK)
        goto error;

    glyphs = (struct rsrc_font_glyph*)rsrc_malloc(sizeof(struct rsrc_font_glyph) * nglyphs);
    if (!glyphs)
        goto error;

    bitmap = (uint8_t*)rsrc_malloc(bmp_width * bmp_height);
    if (!bitmap)
        goto error;

    for (uint32_t glyph_i = 0; glyph_i < nglyphs; ++glyph_i) {
        uint64_t charcode;
        float advance_x, bearing_x, bearing_y;
        uint16_t x, y, glyph_width, glyph_height;
        if (read_bytes(&charcode, sizeof(charcode), &b, &buf_size) != RSRC_OK)
            goto error;
        if (read_bytes(&advance_x, sizeof(advance_x), &b, &buf_size) != RSRC_OK)
            goto error;
        if (read_bytes(&bearing_x, sizeof(bearing_x), &b, &buf_size) != RSRC_OK)
            goto error;
        if (read_bytes(&bearing_y, sizeof(bearing_y), &b, &buf_size) != RSRC_OK)
            goto error;
        if (read_bytes(&x, sizeof(x), &b, &buf_size) != RSRC_OK)
            goto error;
        if (read_bytes(&y, sizeof(y), &b, &buf_size) != RSRC_OK)
            goto error;
        if (read_bytes(&glyph_width, sizeof(glyph_width), &b, &buf_size) != RSRC_OK)
            goto error;
        if (read_bytes(&glyph_height, sizeof(glyph_height), &b, &buf_size) != RSRC_OK)
            goto error;

        struct rsrc_font_glyph* g = &glyphs[glyph_i];

        g->charcode = charcode;
        g->advance_x = advance_x;
        g->bearing_x = bearing_x;
        g->bearing_y = bearing_y;
        g->x = x;
        g->y = y;
        g->width = glyph_width;
        g->height = glyph_height;
    }

    if (read_bytes(bitmap, bmp_width * bmp_height, &b, &buf_size) != RSRC_OK)
        goto error;

    res->nglyphs = nglyphs;
    res->bmp_height = bmp_height;
    res->bmp_width = bmp_width;
    res->line_spacing = line_spacing;
    res->bitmap = bitmap;
    res->glyphs = glyphs;

    return RSRC_OK;

error:
    rsrc_free(glyphs);
    rsrc_free(bitmap);
    return RSRC_FAILURE;
}

void rsrc_font_unload(struct rsrc_font* res)
{
    rsrc_free(res->glyphs);
    rsrc_free(res->bitmap);
    *res = (struct rsrc_font){};
}

enum rsrc_status rsrc_font_save(const struct rsrc_font* res, uint8_t* buffer,
                                uint32_t buf_size)
{
    if (buf_size < rsrc_font_buf_size(res))
        return RSRC_FAILURE;

    if (write_bytes(&rsrc_font_version, sizeof(rsrc_font_version), &buffer,
                    &buf_size) != RSRC_OK)
        goto error;
    if (write_bytes(&res->nglyphs, sizeof(res->nglyphs), &buffer, &buf_size) != RSRC_OK)
        goto error;
    if (write_bytes(&res->bmp_height, sizeof(res->bmp_height), &buffer, &buf_size) != RSRC_OK)
        goto error;
    if (write_bytes(&res->bmp_width, sizeof(res->bmp_width), &buffer, &buf_size) != RSRC_OK)
        goto error;
    if (write_bytes(&res->line_spacing, sizeof(res->line_spacing), &buffer, &buf_size) != RSRC_OK)
        goto error;

    for (uint32_t glyph_i = 0; glyph_i < res->nglyphs; ++glyph_i) {
        const struct rsrc_font_glyph* g = &res->glyphs[glyph_i];
        if (write_bytes(&g->charcode, sizeof(g->charcode),
                        &buffer, &buf_size) != RSRC_OK)
            goto error;
        if (write_bytes(&g->advance_x, sizeof(g->advance_x), &buffer, &buf_size) != RSRC_OK)
            goto error;
        if (write_bytes(&g->bearing_x, sizeof(g->bearing_x), &buffer, &buf_size) != RSRC_OK)
            goto error;
        if (write_bytes(&g->bearing_y, sizeof(g->bearing_y), &buffer, &buf_size) != RSRC_OK)
            goto error;      
        if (write_bytes(&g->x, sizeof(g->x), &buffer, &buf_size) != RSRC_OK)
            goto error;
        if (write_bytes(&g->y, sizeof(g->y), &buffer, &buf_size) != RSRC_OK)
            goto error;
        if (write_bytes(&g->width, sizeof(g->width), &buffer, &buf_size) != RSRC_OK)
            goto error;
        if (write_bytes(&g->height, sizeof(g->height), &buffer, &buf_size) != RSRC_OK)
            goto error;
    }

    if (write_bytes(res->bitmap, res->bmp_width * res->bmp_height, &buffer, &buf_size) != RSRC_OK)
        goto error;

    return RSRC_OK;

error:
    return RSRC_FAILURE;
}

uint64_t rsrc_font_buf_size(const struct rsrc_font* res)
{
    uint32_t metadata_size = sizeof(rsrc_font_version) + 3 * sizeof(uint16_t) + sizeof(float);
    uint32_t glyph_packed_size = sizeof(uint64_t) + 4 * sizeof(uint16_t) + 3 * sizeof(float);
    uint32_t bitmap_size = res->bmp_height * res->bmp_width;
    return metadata_size + res->nglyphs * glyph_packed_size + bitmap_size;
}
