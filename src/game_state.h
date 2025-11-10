
#pragma once

#include "common.h"
#include "constants.h"

struct GameState
{
    f32 cam_pos_x;
    f32 cam_pos_y;
    f32 cam_w;
    f32 cam_aspect_ratio;

    u32 cur_level;

    u32 num_players;
    f32 player_vel_x[MAX_PLAYERS];
    f32 player_vel_y[MAX_PLAYERS];
    f32 player_pos_x[MAX_PLAYERS];
    f32 player_pos_y[MAX_PLAYERS];
    f32 player_pos_z[MAX_PLAYERS];
    u8 player_type[MAX_PLAYERS];
    u8 player_team_id[MAX_PLAYERS];

};

void init_game_state(struct GameState* game_state);

struct GameInput;
void update_game_state(
    struct GameState* game_state,
    const struct GameState* prev_game_state,
    const struct GameInput* game_input);
