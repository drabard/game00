#include "resources_storage.h"

// todo: codegen this

struct storage_pair_mesh {
    str_id id;
    struct rsrc_mesh payload;
};

struct storage_pair_texture {
    str_id id;
    struct rsrc_texture payload;
};

struct storage_pair_font {
    str_id id;
    struct rsrc_font payload;
};

static struct storage_pair_mesh meshes[RSRC_MAX_MESHES];
//static struct storage_pair_texture textures[RSRC_MAX_TEXTURES];
//static struct storage_pair_font fonts[RSRC_MAX_FONTS];

enum rsrc_status rsrc_load(enum rsrc_resource_type type, str_id name, struct rsrc_token* out_token)
{
    uint8_t* file_buf = 0;
    uint32_t size = 0;

    switch (type) {
    case RSRC_MESH: {
        uint64_t max_pairs = RSRC_MAX_MESHES;
        uint64_t idx = name % max_pairs;
        uint64_t col_idx = 0;

        while (col_idx < max_pairs) {
            idx = (idx + col_idx * col_idx) % max_pairs;

            struct storage_pair_mesh* pair = &meshes[idx];
            if (pair->id != 0) {
                // Double loading of the same resource.
                assert(pair->id != name);
                col_idx++;
                continue;
            }

            pair->id = name;
            if (file_load_binary(str_id_resolve(name), &file_buf, &size) != FILE_OK) {
                goto error;
            }
            if (rsrc_mesh_load(&pair->payload, file_buf, size) != RSRC_OK) {
                goto error;
            }

            file_unload_binary(&file_buf);
            return RSRC_OK;
        }

        goto error;
    } break;
    case RSRC_TEXTURE: {
        assert(!"not implemented");
    } break;
    case RSRC_FONT: {
        assert(!"not implemented");
    } break;
    default: {
        assert(!"unknown resource type");
    } break;
    }

error:
    file_unload_binary(&file_buf);
    return RSRC_FAILURE;
}

enum rsrc_status rsrc_unload(struct rsrc_token* token)
{
    switch (token->type) {
    case RSRC_MESH: {
        uint64_t max_pairs = RSRC_MAX_MESHES;
        uint64_t idx = name % max_pairs;
        uint64_t col_idx = 0;

        while (col_idx < max_pairs) {
            idx = (idx + col_idx * col_idx) % max_pairs;

            struct storage_pair_mesh* pair = &meshes[idx];
            if (pair->id != token->name) {
                col_idx++;
                continue;
            }

            pair->id = 0;
            rsrc_mesh_unload(&pair->payload);

            return RSRC_OK;
        }

        goto error;
    } break;
    case RSRC_TEXTURE: {
        assert(!"not implemented");
    } break;
    case RSRC_FONT: {
        assert(!"not implemented");
    } break;
    default: {
        assert(!"unknown resource type");
    } break;
    }

error:
    return RSRC_FAILURE;
}
