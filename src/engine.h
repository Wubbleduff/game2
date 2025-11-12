
#pragma once

#include "common.h"
#include "game_state.h"
#include "path_find.h"
#include "npc.h"

struct Engine
{
    s64 frame_num;

    u32 cur_game_state_idx;
    struct GameState game_states[2];

    struct PathFind path_find;

    u32 num_npcs;
    struct Npc npcs[MAX_PLAYERS];
    u32 last_selected_player_id;

};

void init_engine(struct Engine* engine);

void tick_engine(struct Engine* engine);
