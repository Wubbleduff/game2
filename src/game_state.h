
#pragma once

#include "common.h"

struct GameState
{
    f32 cam_pos_x;
    f32 cam_pos_y;
    f32 cam_w;

    u32 num_quads;
    f32 quad_pos_x[3];
    f32 quad_pos_y[3];
    f32 quad_pos_z[3];
};

void init_game_state(struct GameState* game_state);

void update_game_state(struct GameState* game_state, const struct GameState* prev_game_state);
