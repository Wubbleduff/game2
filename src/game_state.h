
#pragma once

#include "common.h"
#include "constants.h"

#define MAX_BULLETS 8192

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
    s32 player_health[MAX_PLAYERS];
    u8 player_type[MAX_PLAYERS];
    u8 player_team_id[MAX_PLAYERS];

    u32 maybe_flag_held_by_player_id[2];

    u32 num_bullets;
    f32 bullet_vel_x[MAX_BULLETS];
    f32 bullet_vel_y[MAX_BULLETS];
    f32 bullet_pos_x[MAX_BULLETS];
    f32 bullet_pos_y[MAX_BULLETS];
    f32 bullet_prev_pos_x[MAX_BULLETS];
    f32 bullet_prev_pos_y[MAX_BULLETS];
    u8 bullet_team_id[MAX_BULLETS];
};
