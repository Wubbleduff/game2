
#pragma once

#include "common.h"

struct GameState
{
    f32 cam_pos_x;
    f32 cam_pos_y;
    f32 cam_w;
    f32 cam_aspect_ratio;

    f32 player_vel_x;
    f32 player_vel_y;
    f32 player_pos_x;
    f32 player_pos_y;
    f32 player_pos_z;

    f32 player_radius;

    u32 cur_level;
};

void init_game_state(struct GameState* game_state);

struct GameInput;
void update_game_state(
    struct GameState* game_state,
    const struct GameState* prev_game_state,
    const struct GameInput* game_input);
