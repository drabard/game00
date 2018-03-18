static const uint32_t GAME_MAX_ENTITIES = 1024;

struct game_state {
    // RAM Resources
    struct rsrc_mesh cube_mesh;
    struct rsrc_mesh buddha_mesh;
    struct rsrc_texture panda_tex;
    struct rsrc_font roboto_font;

    // Rendering state
    struct gfx_program_storage prog_storage_gfx;

    struct gfx_mesh cube_gfx;
    struct gfx_mesh buddha_gfx;

    struct gfx_font gfx_roboto_font;

    struct gfx_text gfx_fps_txt;
    
    struct cam_noroll camera;

    uint32_t dt_ms;

    v3 light_pos;
};
const uint64_t game_state_size = sizeof(struct game_state);
