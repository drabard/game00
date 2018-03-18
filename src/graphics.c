#include "graphics.h"

static const char* model_uni_name = "model";
static const char* projection_uni_name = "projection";
static const char* view_uni_name = "view";

static const char* text_position_uni_name = "position";

static const char* tex0_sampler_name = "tex0";

static gfx_log_fptr text_log = NULL;

void gfx_set_log(gfx_log_fptr l) { text_log = l; }

static gfx_malloc_fptr gfx_malloc = NULL;
static gfx_free_fptr gfx_free = NULL;
static gfx_realloc_fptr gfx_realloc = NULL;

static float gfx_screen_size[2];

enum gfx_status gfx_init(uint32_t screen_size[2])
{
    gfx_screen_size[0] = (float)screen_size[0];
    gfx_screen_size[1] = (float)screen_size[1];

    if (gpu_init() != GPU_OK)
        goto error;

    return GFX_OK;
error:
    text_log("ERROR: Failed to init graphics.\n");
    return GFX_FAILURE;
}

void gfx_set_mem(gfx_malloc_fptr m, gfx_free_fptr f, gfx_realloc_fptr r)
{
    gfx_malloc = m;
    gfx_free = f;
    gfx_realloc = r;
}

enum gfx_status gfx_compile_shaders(struct gfx_program_storage* storage,
                                    const struct gfx_shader_def* defs,
                                    uint32_t ndefs)
{
    uint32_t curr_i = storage->nshaders;

    storage->nshaders += ndefs;
    storage->shaders = gfx_realloc(storage->shaders,
                                   sizeof(struct gfx_shader) * storage->nshaders);
    if (!storage->shaders)
        goto error;

    for (uint32_t def_i = 0; def_i < ndefs; ++def_i) {
        const struct gfx_shader_def* curr_def = &defs[def_i];

        struct gfx_shader* curr_shader = &storage->shaders[curr_i];
        curr_shader->name = curr_def->name;
        curr_shader->type = curr_def->type;

        if (gpu_compile_shader(&curr_shader->shader,
                               curr_def->type, curr_def->source) != GPU_OK) {
            text_log("ERROR: Failed to load shader \"%s\".\n", curr_shader->name);
            goto error;
        }

        ++curr_i;
    }

    return GFX_OK;
error:
    return GFX_FAILURE;
}

static uint8_t compare_strings(const char* a, const char* b)
{
    while (*a && *b && *a == *b) {
        ++a;
        ++b;
    }

    return (*a == 0 && *b == 0);
}

enum gfx_status gfx_compile_programs(struct gfx_program_storage* storage,
                                     const struct gfx_program_def* defs,
                                     uint32_t ndefs)
{
    uint32_t curr_i = storage->nprograms;

    storage->nprograms += ndefs;
    storage->programs = gfx_realloc(storage->programs,
                                    sizeof(struct gfx_program) * storage->nprograms);
    if (!storage->programs)
        goto error;

    for (uint32_t def_i = 0; def_i < ndefs; ++def_i) {
        const struct gfx_program_def* curr_def = &defs[def_i];
        struct gfx_program* curr_prog = &storage->programs[curr_i];

        curr_prog->name = curr_def->name;

        // Find the relevant shaders
        gpu_shader vs = 0;
        gpu_shader fs = 0;
        for (uint32_t shader_i = 0; shader_i < storage->nshaders; ++shader_i) {
            struct gfx_shader* s = &storage->shaders[shader_i];

            if (vs == 0 && compare_strings(curr_def->vertex_shader_name, s->name)) {
                if (s->type == GFX_VERTEX_SHADER) {
                    vs = s->shader;
                }
            }

            if (fs == 0 && compare_strings(curr_def->fragment_shader_name, s->name)) {
                if (s->type == GFX_FRAGMENT_SHADER) {
                    fs = s->shader;
                }
            }
        }

        if (vs == 0 || fs == 0) {
            text_log("ERROR: Could not find shaders to compile program \"%s\".\n",
                     curr_def->name);
            goto error;
        }

        if (gpu_link_program(&curr_prog->program, vs, fs) != GPU_OK) {
            text_log("ERROR: Could not link program \"%s\".\n",
                     curr_def->name);
            goto error;
        }

        ++curr_i;
    }

    return GFX_OK;

error:
    text_log("ERROR: Could not compile shader programs.\n");

    return GFX_FAILURE;
}

enum gfx_status gfx_get_program(struct gfx_program_storage* storage, const char* program_name,
                                struct gfx_program** program)
{
    struct gfx_program* found = 0;
    for (uint32_t prog_i = 0; prog_i < storage->nprograms; ++prog_i) {
        struct gfx_program* p = &storage->programs[prog_i];
        if (compare_strings(program_name, p->name)) {
            found = p;
            break;
        }
    }

    *program = found;
    if (found) {
        return GFX_OK;
    } else {
        return GFX_FAILURE;
    }
}

enum gfx_status gfx_activate_program(struct gfx_program* program)
{
    if(gpu_activate_program(program->program) != GPU_OK)
    {
        text_log("ERROR: Cannot activate program.\n");
        return GFX_FAILURE;
    }

    return GFX_OK;
}

enum gfx_status gfx_update_view(const m4* view, struct gfx_program* program)
{
    gpu_uniform view_uni;
    if(gpu_get_uniform(program->program, view_uni_name, &view_uni) != GPU_OK)
        goto error;
    if(gpu_set_uniform_m4(view_uni, (float*)view) != GPU_OK)
        goto error;

    return GFX_OK;

error:
    text_log("ERROR: Failed to update view uniform.\n");
    return GFX_FAILURE;
}

enum gfx_status gfx_update_projection(const m4* projection, struct gfx_program* program)
{
    gpu_uniform projection_uni;
    if(gpu_get_uniform(program->program, projection_uni_name, &projection_uni) != GPU_OK)
        goto error;
    if(gpu_set_uniform_m4(projection_uni, (float*)projection) != GPU_OK)
        goto error;

    return GFX_OK;
    
error:
    text_log("ERROR: Failed to update projection uniform.\n");
    return GFX_FAILURE;
}

static const char* texture_unit_sampler_name(uint32_t tex_i)
{
    switch(tex_i)
    {
        case 0:
        {
            return tex0_sampler_name;
        } break;
    }
    return 0;
}

static enum gfx_status create_gpu_mesh(struct gfx_mesh* mesh,
                                        const struct rsrc_mesh* resource)
{
    void* tmp_buf = 0;

    tmp_buf = gfx_malloc(resource->nverts * gpu_max_vert_bytes);
    if (!tmp_buf)
        goto error;

    gpu_vtx_flags_t flags = gpu_pack_verts(tmp_buf, resource->positions, 
        resource->normals, resource->texcoords, resource->nverts);

    if(gpu_vertex_buffer_create(&mesh->vertex_buffer, tmp_buf, flags, resource->indices, resource->nverts, resource->nindices) != GPU_OK) goto error;

    gfx_free(tmp_buf);

    return GFX_OK;

error:
    text_log("ERROR: Couldn't create GPU meshes.\n");
    gfx_free(tmp_buf);
    return GFX_FAILURE;
}

static enum gfx_status create_gpu_textures(struct gfx_mesh* mesh,
                                           const struct rsrc_texture* tex_rsrcs,
                                           uint32_t ntextures)
{
    int32_t tex_i = 0;
    for(; tex_i < (int32_t) ntextures; tex_i++)
    {
        if(gpu_texture_create_RGBA(&mesh->textures[tex_i], tex_rsrcs[tex_i].data, tex_rsrcs[tex_i].width, tex_rsrcs[tex_i].height) != GPU_OK)
            goto error;
    }

    mesh->ntextures = ntextures;

    return GFX_OK;

error:
    text_log("ERROR: Failed to create gpu textures.");

    for(--tex_i; tex_i > 0; --tex_i)
    {
        gpu_texture_destroy(&mesh->textures[tex_i]);
    }

    return GFX_FAILURE;
}

enum gfx_status gfx_mesh_create(struct gfx_mesh* mesh, const struct rsrc_mesh* resource, const struct rsrc_texture* tex_rsrcs, uint32_t ntextures)
{
    *mesh = (struct gfx_mesh){};

    { // Create gpu_meshes
        if (create_gpu_mesh(mesh, resource) != GFX_OK)
            goto error;
    }

    { // Create gpu textures
        mesh->textures = (struct gpu_texture*)gfx_malloc(sizeof(struct gpu_texture) * ntextures);
        if (!mesh->textures)
            goto error;

        if (create_gpu_textures(mesh, tex_rsrcs, ntextures) != GFX_OK)
            goto error;
    }

    return GFX_OK;
error:
    text_log("ERROR: Failed to create graphics mesh.\n");

    gfx_mesh_destroy(mesh);

    return GFX_FAILURE;
}

void gfx_mesh_destroy(struct gfx_mesh* mesh)
{
    gpu_vertex_buffer_destroy(&mesh->vertex_buffer);

    for(uint32_t tex_i = 0; tex_i < mesh->ntextures; ++tex_i)
    {
        gpu_texture_destroy(&mesh->textures[tex_i]);
    }
    gfx_free(mesh->textures);

    *mesh = (struct gfx_mesh){};
}

enum gfx_status gfx_mesh_draw(struct gfx_mesh* mesh, const struct gfx_program* active_program, const m4* transforms, uint32_t ntransforms)
{
    for(uint32_t tex_i = 0; tex_i < mesh->ntextures; ++tex_i)
    {
        if (gpu_texture_bind(&mesh->textures[tex_i], &active_program->program, 1, tex_i, texture_unit_sampler_name(tex_i)) != GPU_OK)
            goto error;
    }

    gpu_uniform model;
    if(gpu_get_uniform(active_program->program, model_uni_name, &model) != GPU_OK)
        goto error;

    for(uint32_t trans_i = 0; trans_i < ntransforms; ++trans_i)
    {
        if(gpu_set_uniform_m4(model, (float*)&transforms[trans_i]) != GPU_OK)
        {
            text_log("ERROR: Cannot set model transform.\n");
            goto error;
        }

        gpu_vertex_buffer_draw(&mesh->vertex_buffer);
    }

    return GFX_OK;

error:
    text_log("ERROR: Failed to draw mesh.\n");
    return GFX_FAILURE;
}

enum gfx_status gfx_font_create(struct gfx_font* fnt, const struct rsrc_font* rsrc)
{
    if(gpu_texture_create_R(&fnt->texture, rsrc->bitmap, rsrc->bmp_width, rsrc->bmp_height) != GPU_OK)
        goto error;
    fnt->resource = rsrc;

    return GFX_OK;

error:
    text_log("ERROR: Failed to create graphics font.\n");
    return GFX_FAILURE;
}

void gfx_font_destroy(struct gfx_font* fnt)
{
    gpu_texture_destroy(&fnt->texture);
    fnt->resource = 0;
}

enum gfx_status gfx_text_create(struct gfx_text* txt, const char* text_ansi, struct gfx_font* font)
{
    *txt = (struct gfx_text){};

    const uint32_t max_supported_characters = 1024;

    float* positions = 0;
    float* texcoords = 0;
    uint32_t* indices = 0;
    void* packed_verts = 0;

    txt->text_ansi = text_ansi;
    txt->font = font;

    { // Layout the text
        const char* c = txt->text_ansi;

        const struct rsrc_font* res = font->resource;

        float tex_width = (float) res->bmp_width;
        float tex_height = (float) res->bmp_height;

        uint32_t text_len = 0;
        {
            uint32_t count = 0;
            while(*(c + count++) != 0) {
                if(count > max_supported_characters)
                {
                    text_log("ERROR[gfx_text_create]: Maximum number of supported characters exceeded.\n");
                    goto error;
                }
            }
            text_len = count;
        }

        // 4 vertices times 3 floats each for each character
        positions = gfx_malloc(sizeof(float) * 12 * text_len );
        // 4 vertices times 3 floats each for each character
        texcoords = gfx_malloc(sizeof(float) * 12 * text_len );
        // 6 indices for each character
        indices = gfx_malloc(sizeof(uint32_t) * 6 * text_len );

        packed_verts = gfx_malloc(text_len * 4 * gpu_max_vert_bytes);

        if( positions == 0 || texcoords == 0 || indices == 0 || 
            packed_verts == 0)
        {
            text_log("ERROR: Out of memory.\n");
            goto error;
        }

        float* curr_pos = positions;
        float* curr_tex = texcoords;
        uint32_t* curr_idx = indices;
        uint32_t idx = 0;

        float start_x = 0.0f;
        float x = start_x;
        float y = -res->line_spacing / (gfx_screen_size[1] * 0.5f);;
        while(*c != '\0')
        {
            if( *c == '\n' )
            {
                y -= res->line_spacing / (gfx_screen_size[1] * 0.5f);
                x = start_x;
                c++;
                continue;
            }

            // Handle characters that don't have glyphs
            const struct rsrc_font_glyph* glyph = 0;
            for(uint32_t glyph_i = 0; glyph_i < res->nglyphs; glyph_i++)
            {
                struct rsrc_font_glyph* curr_glyph = &res->glyphs[glyph_i];
                if( curr_glyph->charcode == (uint64_t)*c )
                {
                    glyph = curr_glyph;
                    break;
                }
            }

            if(glyph == 0)
            {
                text_log("ERROR: Missing glyph for character \'%c\'\n", *c);
                goto error;
            }

            float tex_x = (float) glyph->x / tex_width;
            float tex_y = (float) glyph->y / tex_height;

            float width = (float) glyph->width / (gfx_screen_size[0] * 0.5f);
            float height = (float) glyph->height / (gfx_screen_size[1] * 0.5f);

            float width_tex = (float) glyph->width / tex_width;
            float height_tex = (float) glyph->height / tex_height;
            float advance = glyph->advance_x / (gfx_screen_size[0] * 0.5f);
            float bearing_x = glyph->bearing_x / (gfx_screen_size[0] * 0.5f);
            float bearing_y = glyph->bearing_y / (gfx_screen_size[1] * 0.5f);
            float pos_x = x + bearing_x;
            float pos_y = y + (bearing_y - height);

            x += advance;

            // don't create a quad for space character
            if( *c == ' ' )
            {
                c++;
                continue;
            }

            // bottom left
            *(curr_pos++) = pos_x;
            *(curr_pos++) = pos_y;
            *(curr_pos++) = 0.0f;
            *(curr_tex++) = tex_x;
            *(curr_tex++) = tex_y;
            *(curr_tex++) = 0.0f;            

            // bottom right
            *(curr_pos++) = pos_x + width;
            *(curr_pos++) = pos_y;
            *(curr_pos++) = 0.0f;
            *(curr_tex++) = tex_x + width_tex;
            *(curr_tex++) = tex_y;
            *(curr_tex++) = 0.0f;                        

            // top right
            *(curr_pos++) = pos_x + width;
            *(curr_pos++) = pos_y + height;
            *(curr_pos++) = 0.0f;
            *(curr_tex++) = tex_x + width_tex;
            *(curr_tex++) = tex_y + height_tex;
            *(curr_tex++) = 0.0f;         

            // top left
            *(curr_pos++) = pos_x;
            *(curr_pos++) = pos_y + height;
            *(curr_pos++) = 0.0f;
            *(curr_tex++) = tex_x;
            *(curr_tex++) = tex_y + height_tex;
            *(curr_tex++) = 0.0f;         

            *(curr_idx++) = idx;
            *(curr_idx++) = idx + 1;
            *(curr_idx++) = idx + 2;
            *(curr_idx++) = idx + 2;
            *(curr_idx++) = idx + 3;
            *(curr_idx++) = idx;

            idx += 4;
            
            c++;
        }

        ptrdiff_t nverts = (curr_pos - positions)/3;
        ptrdiff_t nindices = curr_idx - indices;
        gpu_vtx_flags_t flags = gpu_pack_verts(packed_verts, positions, 0, texcoords, nverts );

        gpu_vertex_buffer_create(&txt->quads, packed_verts, flags, indices, nverts, nindices );
    }

    gfx_free(packed_verts);
    gfx_free(positions);
    gfx_free(texcoords);
    gfx_free(indices);

    return GFX_OK;

error:
    gfx_free(packed_verts);
    gfx_free(positions);
    gfx_free(texcoords);
    gfx_free(indices);
    gpu_vertex_buffer_destroy(&txt->quads);

    *txt = (struct gfx_text){};

    text_log("ERROR: Failed to create graphics text: \"%s\".\n", text_ansi);
    return GFX_FAILURE;
}

void gfx_text_destroy(struct gfx_text* txt)
{
    gpu_vertex_buffer_destroy(&txt->quads);
    *txt = (struct gfx_text){};
}

enum gfx_status gfx_text_draw(struct gfx_text* txt, const struct gfx_program* active_program, float screen_x, float screen_y)
{
    gpu_uniform position_uni;
    float position[] = { screen_x, screen_y };

    struct gfx_font* font = txt->font;
   
    if (gpu_get_uniform(active_program->program, text_position_uni_name, &position_uni) != GPU_OK)
        goto error;

    { // Draw text
        if (gpu_texture_bind(&font->texture, &active_program->program, 1, 0, texture_unit_sampler_name(0)) != GPU_OK)
            goto error;

        if (gpu_set_uniform_2f(position_uni, position) != GPU_OK)
            goto error;

        gpu_vertex_buffer_draw(&txt->quads);
    }

    return GFX_OK;

error:
    text_log("ERROR: Cannot draw text.\n");
    return GFX_FAILURE;
}
