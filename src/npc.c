
#include "npc.h"
#include "math.h"
#include "game_input.h"
#include "game_state.h"
#include "path_find.h"

void update_npc(
    struct PlayerInput* player,
    struct Npc* npc,
    struct PathFind* path_find,
    const struct Level* level,
    const struct GameState* game_state,
    const u32 player_id,
    const s64 frame_num)
{
    (void)game_state;

    player->player_move_x = 0.0f;
    player->player_move_y = 0.0f;

    // if(game_state->player_team_id[player_id] == 0)
    // {
    //     npc->target_pos_x = game_state->player_pos_x[0];
    //     npc->target_pos_y = game_state->player_pos_y[0];
    // }

    if(frame_num % 1000 == 0 && game_state->player_team_id[player_id] == 1)
    {
        npc->target_pos_x = (f32)(rand_u32(player_id + 131 + (u32)frame_num) % 128) - 64.0f;
        npc->target_pos_y = (f32)(rand_u32(player_id + 277 + (u32)frame_num) % 64) - 32.0f;
    }

    const v2 player_pos = make_v2(game_state->player_pos_x[player_id], game_state->player_pos_y[player_id]);

    s32 start_x = (s32)round_neg_inf(player_pos.x);
    s32 start_y = (s32)round_neg_inf(player_pos.y);
    s32 end_x   = (s32)round_neg_inf(npc->target_pos_x);
    s32 end_y   = (s32)round_neg_inf(npc->target_pos_y);
    s32 path_x[MAX_PATH_LEN];
    s32 path_y[MAX_PATH_LEN];
    const u32 num_path = run_path_find(
        path_find,
        path_x,
        path_y,
        level,
        start_x,
        start_y,
        end_x,
        end_y);
    if(num_path)
    {
        const v2 next_pos =
            num_path > 1
            ? make_v2((f32)path_x[1] + 0.5f, (f32)path_y[1] + 0.5f)
            : make_v2((f32)path_x[0] + 0.5f, (f32)path_y[0] + 0.5f);
            

        v2 dir = sub_v2(next_pos, player_pos);
        f32 len = length_v2(dir);
        dir = normalize_or_v2(dir, zero_v2());

        // Put on the brakes.
        if(num_path == 3) dir = scale_v2(dir, 0.6f);
        if(num_path == 2) dir = scale_v2(dir, 0.4f);
        if(num_path == 1)
        {
            dir = scale_v2(dir, 3.0f*sq_f32(len) - 2.0f*sq_f32(len)*len);
            dir = scale_v2(dir, 0.3f);
        }

        player->player_move_x = dir.x;
        player->player_move_y = dir.y;
    }
}
