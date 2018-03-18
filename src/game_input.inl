void game_input_reset(struct game_input* inp)
{
    inp->mouse_dx = 0;
    inp->mouse_dy = 0;
}

static void handle_input(struct game_state* game, struct game_input* input)
{
    uint8_t camera_updated = 0;
    float cam_speed = 0.05f;
    float cam_rot_speed = 0.009f;

    v3 pos_off = {};
    float yaw_d = 0.0f;
    float pitch_d = 0.0f;
    if (input->d_down) {
        pos_off.x += cam_speed;
        camera_updated = 1;
    }

    if (input->a_down) {
        pos_off.x -= cam_speed;
        camera_updated = 1;
    }

    if (input->s_down) {
        pos_off.z += cam_speed;
        camera_updated = 1;
    }

    if (input->w_down) {
        pos_off.z -= cam_speed;
        camera_updated = 1;
    }

    if (input->e_down) {
        pos_off.y += cam_speed;
        camera_updated = 1;
    }

    if (input->q_down) {
        pos_off.y -= cam_speed;
        camera_updated = 1;
    }

    if (input->mouse_dx) {
        yaw_d += cam_rot_speed * input->mouse_dx;
        camera_updated = 1;
    }

    if (input->mouse_dy) {
        pitch_d += cam_rot_speed * input->mouse_dy;
        camera_updated = 1;
    }

    if (camera_updated != 0) {
        cam_noroll_update(&game->camera, yaw_d, pitch_d, pos_off);
    }

    if(input->i_down) {
        game->light_pos.y += 0.05f;
    }

    if(input->j_down) {
        game->light_pos.x -= 0.05f;
    }

    if(input->k_down) {
        game->light_pos.y -= 0.05f;
    }

    if(input->l_down) {
        game->light_pos.x += 0.05f;
    }

    if(input->u_down) {
        game->light_pos.z += 0.05f;
    }

    if(input->o_down) {
        game->light_pos.z -= 0.05f;
    }
}
