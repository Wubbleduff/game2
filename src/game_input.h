
#pragma once

#include "common.h"
#include "constants.h"

struct PlayerInput
{
    f32 cursor_pos_x;
    f32 cursor_pos_y;

    f32 player_move_x;
    f32 player_move_y;
};

struct GameInput
{
    u32 num_players;
    struct PlayerInput player_input[MAX_PLAYERS];
};

