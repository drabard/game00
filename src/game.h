#pragma once

#include <stdint.h>

enum game_status { GAME_OK = 0,
                   GAME_FAILURE };

struct game_settings {
    const char* name;
    uint32_t screen_size[2];
    uint32_t max_fps;
};

struct game_input {
    uint8_t d_down;
    uint8_t a_down;
    uint8_t w_down;
    uint8_t s_down;
    uint8_t q_down;
    uint8_t e_down;
    uint8_t i_down;
    uint8_t k_down;
    uint8_t j_down;
    uint8_t l_down;
    uint8_t u_down;
    uint8_t o_down;

    int32_t mouse_dx;
    int32_t mouse_dy;
};

void game_input_reset(struct game_input* inp);

struct game_state;
extern const uint64_t game_state_size;

enum game_status game_init(struct game_state* game, struct game_settings* settings);
void game_deinit(struct game_state* game);
void game_update(struct game_state* game, uint32_t dt_ms, struct game_input* input);
void game_draw(struct game_state* game);
