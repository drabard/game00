static enum game_status load_meshes(struct game_state* game)
{
    uint8_t* buf = 0;
    uint32_t buf_size = 0;
    if (file_load_binary("res/meshes/box.mesh", &buf, &buf_size) != FILE_OK)
        goto error;

    if (rsrc_mesh_load(&game->cube_mesh, buf, buf_size) != RSRC_OK)
        goto error;

    if (file_load_binary("res/meshes/buddha.mesh", &buf, &buf_size) != FILE_OK)
        goto error;

    if (rsrc_mesh_load(&game->buddha_mesh, buf, buf_size) != RSRC_OK)
        goto error;

    file_unload_binary(&buf);

    return GAME_OK;

error:
    file_unload_binary(&buf);
    game_log("ERROR: Failed to load meshes.\n");

    rsrc_mesh_unload(&game->cube_mesh);
    rsrc_mesh_unload(&game->buddha_mesh);

    return GAME_FAILURE;
}

static enum game_status load_fonts(struct game_state* game)
{
    uint8_t* buf = 0;
    uint32_t buf_size = 0;
    if (file_load_binary("res/fonts/roboto.fnt", &buf, &buf_size) != FILE_OK)
        goto error;

    if (rsrc_font_load(&game->roboto_font, buf, buf_size) != RSRC_OK)
        goto error;

    file_unload_binary(&buf);

    return GAME_OK;

error:
    file_unload_binary(&buf);
    rsrc_font_unload(&game->roboto_font);

    game_log("ERROR: Failed to load fonts.\n");

    return GAME_FAILURE;
}

static enum game_status load_textures(struct game_state* game)
{
    uint8_t* buf = 0;
    uint32_t buf_size = 0;
    if (file_load_binary("res/textures/panda.png", &buf, &buf_size) != FILE_OK)
        goto error;

    if (rsrc_texture_load(&game->panda_tex, buf, buf_size) != RSRC_OK)
        goto error;

    file_unload_binary(&buf);

    return GAME_OK;

error:
    game_log("ERROR: Failed to load textures.\n");

    file_unload_binary(&buf);
    rsrc_texture_unload(&game->panda_tex);

    return GAME_FAILURE;
}

static enum game_status init_shaders(struct game_state* game)
{
    const char* v_ssrc = 0;
    uint32_t v_ssrc_size = 0;
    const char* f_ssrc = 0;
    uint32_t f_ssrc_size = 0;

    struct gfx_shader_def defs[2];

    { // Basic mesh program
        if (file_load_text("res/shaders/basic.vs", &v_ssrc, &v_ssrc_size) != FILE_OK)
            goto error;
        if (file_load_text("res/shaders/basic.fs", &f_ssrc, &f_ssrc_size) != FILE_OK)
            goto error;

        defs[0] = (struct gfx_shader_def){.name = "basic",
                                          .source = v_ssrc,
                                          .type = GFX_VERTEX_SHADER };

        defs[1] = (struct gfx_shader_def){.name = "basic",
                                          .source = f_ssrc,
                                          .type = GFX_FRAGMENT_SHADER };

        if (gfx_compile_shaders(&game->prog_storage_gfx, defs, 2) != GFX_OK)
            goto error;

        file_unload_text(&v_ssrc);
        file_unload_text(&f_ssrc);

        struct gfx_program_def basic_prog_def = (struct gfx_program_def){
            .name = "basic",
            .vertex_shader_name = "basic",
            .fragment_shader_name = "basic"
        };

        if (gfx_compile_programs(&game->prog_storage_gfx, &basic_prog_def, 1) != GFX_OK)
            goto error;
    }

    { // Basic text program
        if (file_load_text("res/shaders/text.vs", &v_ssrc, &v_ssrc_size) != FILE_OK)
            goto error;
        if (file_load_text("res/shaders/text.fs", &f_ssrc, &f_ssrc_size) != FILE_OK)
            goto error;

        defs[0] = (struct gfx_shader_def){.name = "text",
                                          .source = v_ssrc,
                                          .type = GFX_VERTEX_SHADER };

        defs[1] = (struct gfx_shader_def){.name = "text",
                                          .source = f_ssrc,
                                          .type = GFX_FRAGMENT_SHADER };

        if (gfx_compile_shaders(&game->prog_storage_gfx, defs, 2) != GFX_OK)
            goto error;

        file_unload_text(&v_ssrc);
        file_unload_text(&f_ssrc);

        struct gfx_program_def text_prog_def = (struct gfx_program_def){
            .name = "text",
            .vertex_shader_name = "text",
            .fragment_shader_name = "text"
        };

        if (gfx_compile_programs(&game->prog_storage_gfx, &text_prog_def, 1) != GFX_OK)
            goto error;
    }

    return GAME_OK;

error:
    file_unload_text(&v_ssrc);
    file_unload_text(&f_ssrc);

    return GAME_FAILURE;
}

enum game_status game_init(struct game_state* game, struct game_settings* settings)
{
    // Init logging
    cam_set_log(game_log);
    rsrc_set_log(game_log);
    gpu_set_log(game_log);
    gfx_set_log(game_log);

    // Resources
    if (rsrc_init() != RSRC_OK)
        goto error;
    rsrc_set_mem(mem_alloc, mem_free, mem_realloc);

    // Graphics
    if (gfx_init(settings->screen_size) != GFX_OK)
        goto error;
    gfx_set_mem(mem_alloc, mem_free, mem_realloc);

    { // Load resources
        if (load_meshes(game) != GAME_OK)
            goto error;
        if (load_textures(game) != GAME_OK)
            goto error;
        if (load_fonts(game) != GAME_OK)
            goto error;
    }

    if (init_shaders(game) != GAME_OK)
        goto error;

    { // Create render groups
        struct gfx_program* basic_prog = 0;

        if (gfx_get_program(&game->prog_storage_gfx, "basic", &basic_prog) != GFX_OK)
            goto error;

        if (gfx_mesh_create(&game->buddha_gfx, &game->buddha_mesh, 0, 0)
            != GFX_OK)
            goto error;

        if (gfx_mesh_create(&game->cube_gfx, &game->cube_mesh, &game->panda_tex, 1) != GFX_OK)
            goto error;
    }

    { // Create font for debug rendering
        struct gfx_program* prog = 0;

        if (gfx_get_program(&game->prog_storage_gfx, "text", &prog) != GFX_OK)
            goto error;

        if (gfx_font_create(&game->gfx_roboto_font, &game->roboto_font) != GFX_OK)
            goto error;
    }

    { // Create test text to display
        if (gfx_text_create(&game->gfx_fps_txt, "FPS: 0", &game->gfx_roboto_font) != GFX_OK) {
            goto error;
        }
    }

    { // Init camera
        cam_noroll_init(&game->camera, (v3){.x = 0.0f, .y = 0.0f, .z = -3.0f },
                        0.017f * 180.0f, 0.0f);
    }

    game->light_pos = (v3){.x = 0.0f, .y = 0.0f, .z = 0.0f };

    return GAME_OK;

error:
    game_log("ERROR: Failed to initialize game.\n");
    return GAME_FAILURE;
}

void game_deinit(struct game_state* game)
{
    // Deinit drawables
    gfx_mesh_destroy(&game->cube_gfx);
    gfx_mesh_destroy(&game->buddha_gfx);

    gfx_font_destroy(&game->gfx_roboto_font);
    gfx_text_destroy(&game->gfx_fps_txt);

    // Release resources
    rsrc_mesh_unload(&game->cube_mesh);
    rsrc_mesh_unload(&game->buddha_mesh);

    rsrc_texture_unload(&game->panda_tex);
    rsrc_font_unload(&game->roboto_font);
}

void game_update(struct game_state* game, uint32_t dt_ms, struct game_input* input)
{
    game->dt_ms = dt_ms;
    handle_input(game, input);

    { // create the fps text
        static const uint32_t nrecords = 30;
        static uint32_t record_i = 0;
        static float records[nrecords] = {};

        records[record_i] = 1000.0f / (float)dt_ms;
        record_i = (record_i + 1) % nrecords;

        if (record_i == (nrecords - 1)) {
            float avg = 0.0f;
            for (uint32_t i = 0; i < nrecords; i++) {
                avg += records[i];
            }
            avg /= nrecords;

            char str[16];
            snprintf(str, sizeof(str) / sizeof(str[0]), "FPS: %.2f", avg);

            gfx_text_destroy(&game->gfx_fps_txt);
            if (gfx_text_create(&game->gfx_fps_txt, str, &game->gfx_roboto_font) != GFX_OK) {
                game_log("ERROR: Cannot create FPS text.\n");
                return;
            }
        }
    }
}

void game_draw(struct game_state* game)
{
    { // Draw meshes
        struct gfx_program* basic_program;
        if (gfx_get_program(&game->prog_storage_gfx, "basic", &basic_program) != GFX_OK)
            goto error;

        gfx_activate_program(basic_program);

        m4 view;
        m4_view_from_quat(&view, game->camera.orientation, game->camera.position);
        gfx_update_view(&view, basic_program);

        // This can be done once on relevant programs' init.
        m4 projection;
        m4_perspective(&projection, 90.0f, 16.0f / 9.0f,
                       0.01f, 100.0f);
        gfx_update_projection(&projection, basic_program);

        {
            gpu_uniform light_pos;
            gpu_get_uniform(basic_program->program, "light_pos", &light_pos);

            gpu_set_uniform_3f(light_pos, game->light_pos.data);
        }

        m4 cube_model;
        m4 buddha_model;
        m4 light_model;
        m4_unit(&cube_model);
        m4_unit(&buddha_model);
        m4_unit(&light_model);

        m4_set_translation(&light_model, game->light_pos);
        m4_set_scale(&light_model, (struct v3){.x = 0.1f, .y = 0.1f, .z = 0.1f });

        m4_set_translation(&cube_model, (struct v3){.x = 1.0f });

        m4_set_translation(&buddha_model, (struct v3){.x = -1.0f, .y = -1.0f });
        m4_set_scale(&buddha_model, (struct v3){.x = 0.3f, .y = 0.3f, .z = 0.3f });

        gfx_mesh_draw(&game->cube_gfx, basic_program, &cube_model, 1);
        gfx_mesh_draw(&game->buddha_gfx, basic_program, &buddha_model, 1);
        gfx_mesh_draw(&game->cube_gfx, basic_program, &light_model, 1);
    }

    { // Draw on-screen text
        struct gfx_program* text_program;
        if (gfx_get_program(&game->prog_storage_gfx, "text", &text_program) != GFX_OK)
            goto error;

        gfx_activate_program(text_program);

        gfx_text_draw(&game->gfx_fps_txt, text_program, -1.0f, 1.0f);
    }

    return;

error:
    game_log("ERROR: Cannot draw game.\n");
}