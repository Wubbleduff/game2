
#pragma once

#include "common.h"
#include "constants.h"

struct Npc
{
    f32 target_pos_x;
    f32 target_pos_y;
};

struct PlayerInput;
struct GameState;
struct PathFind;
struct Level;
void update_npc(
    struct PlayerInput* player,
    struct Npc* npc,
    struct PathFind* path_find,
    const struct Level* level,
    const struct GameState* game_state,
    const u32 player_id,
    const s64 frame_num);
