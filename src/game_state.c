
#include "game_state.h"
#include "input.h"

void init_game_state(struct GameState* game_state)
{
    game_state->cam_pos_x = 0.0f;
    game_state->cam_pos_y = 0.0f;
    game_state->cam_w = 20.0f;

    game_state->num_quads = 0;
}

void update_game_state(struct GameState* game_state, const struct GameState* prev_game_state)
{
    f32 cam_pos_x = prev_game_state->cam_pos_x;
    f32 cam_pos_y = prev_game_state->cam_pos_y;

    cam_pos_x += (f32)is_keyboard_key_down(KB_D) * 0.1f;
    cam_pos_x -= (f32)is_keyboard_key_down(KB_A) * 0.1f;
    cam_pos_y += (f32)is_keyboard_key_down(KB_W) * 0.1f;
    cam_pos_y -= (f32)is_keyboard_key_down(KB_S) * 0.1f;

    game_state->num_quads = 1;
    {
        game_state->quad_pos_x[0] = 0.0f;
        game_state->quad_pos_y[0] = 0.0f;
        game_state->quad_pos_z[0] = 0.5f;
    }

    game_state->cam_pos_x = cam_pos_x;
    game_state->cam_pos_y = cam_pos_y;
}
