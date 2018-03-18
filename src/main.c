#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "GL/gl3w.h"
#include "SDL2/SDL.h"

#include "game.h"

#define ARR_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO);

    struct game_settings settings;
    { // Fill in game settings
        settings.name = "game00";
        settings.screen_size[0] = 1366;
        settings.screen_size[1] = 768;
    }

    SDL_Window* window;
    SDL_GLContext glcontext;
    { // Initialize window with OpenGL 3.3 core context
        window = SDL_CreateWindow(settings.name,
                                  SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED,
                                  settings.screen_size[0], 
                                  settings.screen_size[1],
                                  SDL_WINDOW_OPENGL);

        if (window == NULL) {
            printf("ERROR: Could not create window: %s\n", SDL_GetError());
            return 1;
        }

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        
        glcontext = SDL_GL_CreateContext(window);
        SDL_GL_MakeCurrent(window, glcontext);

        SDL_GL_SetSwapInterval(1); 

        if (gl3wInit()) {
            printf("ERROR: Cannot initialize gl3w.\n");
            return 1;
        }

        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);

        printf("OpenGL version: %s\n", glGetString(GL_VERSION));
    }

    struct game_state* game;
    { // Allocate and initialize game state
        game = (struct game_state*)calloc(1, game_state_size);

        if (game_init(game, &settings) != GAME_OK) {
            return 1;
        }
    }

    struct game_input input = {};

    SDL_Event event;
    int32_t prev_mouse_x;
    int32_t prev_mouse_y;
    SDL_PumpEvents();
    SDL_GetMouseState(&prev_mouse_x, &prev_mouse_y);

    uint8_t done = 0;
    uint32_t current_time = SDL_GetTicks();    
    uint32_t last_time = current_time - 17;    
    uint32_t dt_ms;
    while (!done) {
        current_time = SDL_GetTicks();
        dt_ms = current_time - last_time;

        game_input_reset(&input);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done = 1;
                break;
            } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                switch (event.key.keysym.sym) {
                case SDLK_d: {
                    if (event.key.state == SDL_PRESSED)
                        input.d_down = 1;
                    else
                        input.d_down = 0;
                } break;
                case SDLK_a: {
                    if (event.key.state == SDL_PRESSED)
                        input.a_down = 1;
                    else
                        input.a_down = 0;
                } break;
                case SDLK_w: {
                    if (event.key.state == SDL_PRESSED)
                        input.w_down = 1;
                    else
                        input.w_down = 0;
                } break;
                case SDLK_s: {
                    if (event.key.state == SDL_PRESSED)
                        input.s_down = 1;
                    else
                        input.s_down = 0;
                } break;
                case SDLK_q: {
                    if (event.key.state == SDL_PRESSED)
                        input.q_down = 1;
                    else
                        input.q_down = 0;
                } break;
                case SDLK_e: {
                    if (event.key.state == SDL_PRESSED)
                        input.e_down = 1;
                    else
                        input.e_down = 0;
                } break;
                case SDLK_i: {
                    if (event.key.state == SDL_PRESSED)
                        input.i_down = 1;
                    else
                        input.i_down = 0;
                } break;
                case SDLK_j: {
                    if (event.key.state == SDL_PRESSED)
                        input.j_down = 1;
                    else
                        input.j_down = 0;
                } break;
                case SDLK_k: {
                    if (event.key.state == SDL_PRESSED)
                        input.k_down = 1;
                    else
                        input.k_down = 0;
                } break;
                case SDLK_l: {
                    if (event.key.state == SDL_PRESSED)
                        input.l_down = 1;
                    else
                        input.l_down = 0;
                } break;
                case SDLK_u: {
                    if (event.key.state == SDL_PRESSED)
                        input.u_down = 1;
                    else
                        input.u_down = 0;
                } break;
                case SDLK_o: {
                    if (event.key.state == SDL_PRESSED)
                        input.o_down = 1;
                    else
                        input.o_down = 0;
                } break;
            };
            } else if (event.type == SDL_MOUSEMOTION) {
                input.mouse_dx = event.motion.x - prev_mouse_x;
                input.mouse_dy = event.motion.y - prev_mouse_y;
                prev_mouse_x = event.motion.x;
                prev_mouse_y = event.motion.y;
            }
        }

        game_update(game, dt_ms, &input);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        game_draw(game);

        SDL_GL_SwapWindow(window);
        
        last_time = current_time;
    }

    game_deinit(game);

    free(game);

    SDL_GL_DeleteContext(glcontext);

    SDL_DestroyWindow(window);
    window = NULL;

    SDL_Quit();
    return 0;
}
