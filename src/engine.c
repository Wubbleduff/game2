
#include "engine.h"
#include "game_input.h"
#include "math.h"
#include "constants.h"
#include "platform.h"

#include "level0.h"

#include "debug_draw.h"



// NOTE:
// This will consider overlapping points and line segments as intersecting.
static inline u8 intersect_line_segments_2d(
    const v2 a0,
    const v2 a1,
    const v2 b0,
    const v2 b1)
{
    u8 result = 1;

    {
        const v2 p0 = a0;
        const v2 p1 = a1;
        const v2 q0 = b0;
        const v2 q1 = b1;

        const v2 p01 = sub_v2(p1, p0);
        const v2 n = make_v2(-p01.y, p01.x);

        const f32 d_q0 = dot_v2(sub_v2(q0, p0), n);
        const f32 d_q1 = dot_v2(sub_v2(q1, p0), n);

        result &= sign_f32(d_q0) ^ sign_f32(d_q1);
        result |= d_q0 == 0.0f;
    }

    {
        const v2 p0 = b0;
        const v2 p1 = b1;
        const v2 q0 = a0;
        const v2 q1 = a1;

        const v2 p01 = sub_v2(p1, p0);
        const v2 n = make_v2(-p01.y, p01.x);

        const f32 d_q0 = dot_v2(sub_v2(q0, p0), n);
        const f32 d_q1 = dot_v2(sub_v2(q1, p0), n);

        result &= sign_f32(d_q0) ^ sign_f32(d_q1);
        result |= d_q0 == 0.0f;
    }

    return result;
}



static void init_game_state(struct GameState* game_state)
{
    game_state->cam_pos_x = 0.0f;
    game_state->cam_pos_y = 0.0f;
    game_state->cam_w = 60.0f;
    game_state->cam_aspect_ratio = platform_get_screen_aspect_ratio();

    game_state->cur_level = 0;

    {
        u32 num = 0;
        for(u64 i = 0; i < 16; i++)
        {
            v2 center = make_v2(LEVEL0_LEFT + 15.0f, 0.0f);
            v2 pos = add_v2(center, scale_v2(make_v2((f32)(i % 4), (f32)(i / 4) - 2.0f), 1.2f));
            game_state->player_vel_x[num] = 0.0f;
            game_state->player_vel_y[num] = 0.0f;
            game_state->player_pos_x[num] = pos.x;
            game_state->player_pos_y[num] = pos.y;
            game_state->player_health[num] = 100;
            game_state->player_type[num] = 0;
            game_state->player_team_id[num] = 0;
            num++;
        }

        for(u64 i = 0; i < 16; i++)
        {
            v2 center = make_v2(LEVEL0_RIGHT - 15.0f - 4.0f, 0.0f);
            v2 pos = add_v2(center, scale_v2(make_v2((f32)(i % 4), (f32)(i / 4) - 2.0f), 1.2f));
            game_state->player_vel_x[num] = 0.0f;
            game_state->player_vel_y[num] = 0.0f;
            game_state->player_pos_x[num] = pos.x;
            game_state->player_pos_y[num] = pos.y;
            game_state->player_health[num] = 100;
            game_state->player_type[num] = 0;
            game_state->player_team_id[num] = 1;
            num++;
        }
        game_state->num_players = num;

        game_state->player_id_has_flag[0] = u32_MAX;
        game_state->player_id_has_flag[1] = u32_MAX;
    }

    game_state->num_bullets = 0;
}

static void update_physics(
    struct GameState* game_state,
    u8* bullet_is_dead,
    const struct GameInput* game_input)
{
    const u32 num_iterations = 16;
    const f32 sub_dt = (f32)FRAME_DURATION_NS * (1.0f / 1000000000.0f) * (1.0f / (f32)num_iterations);
    const f32 max_accel = 4000.0f / (f32)num_iterations;
    const f32 drag = -250.0f / (f32)num_iterations;

    const u32 num_players = game_state->num_players;
    const f32 player_radius = 0.5f;
    f32* player_vel_x = game_state->player_vel_x;
    f32* player_vel_y = game_state->player_vel_y;
    f32* player_pos_x = game_state->player_pos_x;
    f32* player_pos_y = game_state->player_pos_y;
    s32* player_health = game_state->player_health;
    u8* player_team_id = game_state->player_team_id;

    const u32 num_bullets = game_state->num_bullets;
    f32* bullet_vel_x = game_state->bullet_vel_x;
    f32* bullet_vel_y = game_state->bullet_vel_y;
    f32* bullet_pos_x = game_state->bullet_pos_x;
    f32* bullet_pos_y = game_state->bullet_pos_y;
    f32* bullet_prev_pos_x = game_state->bullet_prev_pos_x;
    f32* bullet_prev_pos_y = game_state->bullet_prev_pos_y;
    u8* bullet_team_id = game_state->bullet_team_id;

    ASSERT(game_state->cur_level == 0, "TODO levels");
    const struct Level* level = &LEVEL0;

    // Iteratively update physics.
    for(u32 iteration = 0; iteration < num_iterations; iteration++)
    {
        // Note: The order with which we process kinematics and collisions is important.

        // 1. Integrate forces into player velocity.
        for(u32 player_id = 0; player_id < num_players; player_id++)
        {
            const struct PlayerInput* player_input = &game_input->player_input[player_id];
            const v2 move_dir = make_v2(player_input->move_x, player_input->move_y);
            v2 player_vel = make_v2(player_vel_x[player_id], player_vel_y[player_id]);

            v2 accel = zero_v2();

            accel = scale_v2(move_dir, max_accel);

            // Drag.
            // a' = a + v * d;
            accel = add_v2(accel, scale_v2(player_vel, drag));

            // v' = a*t + v
            player_vel = add_v2(player_vel, scale_v2(accel, sub_dt));

            player_vel_x[player_id] = player_vel.x;
            player_vel_y[player_id] = player_vel.y;
        }
        
        // 2. Resolve bullet-wall collisions.
        for(u32 i_bullet = 0; i_bullet < num_bullets; i_bullet++)
        {
            for(u32 i_wall = 0; i_wall < level->num_walls; i_wall++)
            {
                const v2 bullet_prev_pos = make_v2(bullet_prev_pos_x[i_bullet], bullet_prev_pos_y[i_bullet]);
                const v2 bullet_pos = make_v2(bullet_pos_x[i_bullet], bullet_pos_y[i_bullet]);
        
                const struct LevelWallGeometry* wall = &level->walls[i_wall];
                const v2 wall_bl = make_v2((f32)wall->x,                  (f32)wall->y);
                const v2 wall_br = make_v2((f32)(wall->x + (s32)wall->w), (f32)wall->y);
                const v2 wall_tl = make_v2((f32)wall->x,                  (f32)(wall->y + (s32)wall->h));
                const v2 wall_tr = make_v2((f32)(wall->x + (s32)wall->w), (f32)(wall->y + (s32)wall->h));
        
                u32 hit = 0;
        
                // TODO(mfritz):
                // This is not correct! While playing, an assert fired that bullet went out of bounds.
                hit |= intersect_line_segments_2d(bullet_prev_pos, bullet_pos, wall_bl, wall_br);
                hit |= intersect_line_segments_2d(bullet_prev_pos, bullet_pos, wall_br, wall_tr);
                hit |= intersect_line_segments_2d(bullet_prev_pos, bullet_pos, wall_tr, wall_tl);
                hit |= intersect_line_segments_2d(bullet_prev_pos, bullet_pos, wall_tl, wall_bl);
        
                hit &= !(bullet_is_dead[i_bullet]);
        
                if(hit)
                {
                    bullet_is_dead[i_bullet] = 1;
                }
            }
        }

        // 3. Resolve bullet-player collisions.
        for(u32 i_bullet = 0; i_bullet < num_bullets; i_bullet++)
        {
            for(u32 player_id = 0; player_id < num_players; player_id++)
            {
                const v2 bullet_prev_pos = make_v2(bullet_prev_pos_x[i_bullet], bullet_prev_pos_y[i_bullet]);
                const v2 bullet_pos = make_v2(bullet_pos_x[i_bullet], bullet_pos_y[i_bullet]);

                const v2 player_pos = make_v2(player_pos_x[player_id], player_pos_y[player_id]);

                u32 hit = 1;
                f32 dist = distance_point_line_segment_2d(player_pos, bullet_prev_pos, bullet_pos);
                hit &= dist < player_radius;
                hit &= bullet_team_id[i_bullet] != player_team_id[player_id];
                hit &= !(bullet_is_dead[i_bullet]);
                if(hit)
                {
                    player_vel_x[player_id] += bullet_vel_x[i_bullet] * 0.1f;
                    player_vel_y[player_id] += bullet_vel_y[i_bullet] * 0.1f;
                    player_health[player_id] = max_s32(player_health[player_id] - 25, 0);
                    bullet_is_dead[i_bullet] = 1;
                }
            }
        }

        // 4. Resolve player-player collisions.
        for(u32 a_id = 0; a_id < num_players; a_id++)
        {
            const v2 a_pos = make_v2(player_pos_x[a_id], player_pos_y[a_id]);
            v2 a_vel = make_v2(player_vel_x[a_id], player_vel_y[a_id]);
            const f32 a_radius = player_radius;

            // Note: This technically makes us dependent on the order of updated players. We could fix this by introducing an intermediate buffer for
            //       position and velocity.
            for(u32 b_id = 0; b_id < num_players; b_id++)
            {
                const v2 b_pos = make_v2(player_pos_x[b_id], player_pos_y[b_id]);
                v2 b_vel = make_v2(player_vel_x[b_id], player_vel_y[b_id]);
                const f32 b_radius = player_radius;

                const v2 n = sub_v2(a_pos, b_pos);
                const v2 rel_vel = sub_v2(a_vel, b_vel);
                if(dot_v2(n, n) < sq_f32(a_radius + b_radius) &&
                    dot_v2(rel_vel, n) < 0.0f)
                {
                    const f32 j = dot_v2(scale_v2(rel_vel, -1.0f), n) / (dot_v2(n, n) * 2.0f);

                    a_vel = add_v2(a_vel, scale_v2(n, j));
                    b_vel = add_v2(b_vel, scale_v2(n, -j));

                    // Only need to write b_vel out because a_val is cached for this player and will be written at the very end.
                    player_vel_x[b_id] = b_vel.x;
                    player_vel_y[b_id] = b_vel.y;
                }
            }

            player_vel_x[a_id] = a_vel.x;
            player_vel_y[a_id] = a_vel.y;
        }

        // 5. Resolve player-wall collisions.
        //    Do this after all player-player collisions so it's harder for players to move into walls.
        for(u32 player_id = 0; player_id < num_players; player_id++)
        {
            const v2 player_pos = make_v2(player_pos_x[player_id], player_pos_y[player_id]);
            const f32 player_r = player_radius;
            v2 player_vel = make_v2(player_vel_x[player_id], player_vel_y[player_id]);

            for(u32 i_wall = 0; i_wall < level->num_walls; i_wall++)
            {
                const struct LevelWallGeometry* wall = &level->walls[i_wall];
                const f32 wall_left = (f32)wall->x;
                const f32 wall_right = (f32)(wall->x + (s32)wall->w);
                const f32 wall_bottom = (f32)wall->y;
                const f32 wall_top = (f32)(wall->y + (s32)wall->h);

                v2 clamped_pos = player_pos;
                clamped_pos.x = clamp_f32(player_pos.x, wall_left, wall_right);
                clamped_pos.y = clamp_f32(player_pos.y, wall_bottom, wall_top);

                const v2 n = sub_v2(player_pos, clamped_pos);
                if(dot_v2(n, n) < sq_f32(player_r) &&
                   dot_v2(n, player_vel) < 0.0f)
                {
                    const f32 j = dot_v2(scale_v2(player_vel, -1.0f), n) / dot_v2(n, n);
                    player_vel = add_v2(player_vel, scale_v2(n, j));
                }
            }

            player_vel_x[player_id] = player_vel.x;
            player_vel_y[player_id] = player_vel.y;
        }

        // 6. Integrate velocity into position.
        for(u32 player_id = 0; player_id < num_players; player_id++)
        {
            player_pos_x[player_id] += player_vel_x[player_id] * sub_dt;
            player_pos_y[player_id] += player_vel_y[player_id] * sub_dt;

            ASSERT(player_pos_x[player_id] >= -1000.0f, "Bullet out of bounds.");
            ASSERT(player_pos_x[player_id] < 1000.0f, "Bullet out of bounds.");
            ASSERT(player_pos_y[player_id] >= -1000.0f, "Bullet out of bounds.");
            ASSERT(player_pos_y[player_id] < 1000.0f, "Bullet out of bounds.");
        }
        for(u32 i = 0; i < num_bullets; i++)
        {
            bullet_pos_x[i] += bullet_vel_x[i] * sub_dt;
            bullet_pos_y[i] += bullet_vel_y[i] * sub_dt;

            ASSERT(bullet_pos_x[i] >= -1000.0f, "Bullet out of bounds.");
            ASSERT(bullet_pos_x[i] < 1000.0f, "Bullet out of bounds.");
            ASSERT(bullet_pos_y[i] >= -1000.0f, "Bullet out of bounds.");
            ASSERT(bullet_pos_y[i] < 1000.0f, "Bullet out of bounds.");
        }
    }
}

static void assign_bullet(
    struct GameState* game_state,
    const u64 idx,
    const f32 vel_x,
    const f32 vel_y,
    const f32 pos_x,
    const f32 pos_y,
    const f32 prev_pos_x,
    const f32 prev_pos_y,
    const u8 team_id)
{
    game_state->bullet_vel_x[idx] = vel_x;
    game_state->bullet_vel_y[idx] = vel_y;
    game_state->bullet_pos_x[idx] = pos_x;
    game_state->bullet_pos_y[idx] = pos_y;
    game_state->bullet_prev_pos_x[idx] = prev_pos_x;
    game_state->bullet_prev_pos_y[idx] = prev_pos_y;
    game_state->bullet_team_id[idx] = team_id;
}

static void add_bullet(
    struct GameState* game_state,
    const f32 vel_x,
    const f32 vel_y,
    const f32 pos_x,
    const f32 pos_y,
    const f32 prev_pos_x,
    const f32 prev_pos_y,
    const u8 team_id)
{
    const u32 num = game_state->num_bullets;
    ASSERT(num < MAX_BULLETS, "Bullet overflow.");
    assign_bullet(
        game_state,
        num,
        vel_x,
        vel_y,
        pos_x,
        pos_y,
        prev_pos_x,
        prev_pos_y,
        team_id
    );
    game_state->num_bullets = num + 1;
}



void init_engine(struct Engine* engine)
{
    engine->frame_num = 0;

    for(u64 i = 0; i < ARRAY_COUNT(engine->game_states); i++)
    {
        init_game_state(&engine->game_states[i]);
    }
    engine->cur_game_state_idx = 0;

    init_path_find(&engine->path_find, &LEVEL0);

    {
        const struct GameState* game_state = &engine->game_states[engine->cur_game_state_idx];

        engine->num_npcs = 32;
        for(u64 i = 0; i < 32; i++)
        {
            struct Npc* npc = &engine->npcs[i];
            if(game_state->player_team_id[i] == 0)
            {
                npc->target_pos_x = game_state->player_pos_x[i];
                npc->target_pos_y = game_state->player_pos_y[i];
            }
            else
            {
                npc->target_pos_x = (f32)(rand_u32((u32)i + 131) % 128) - 64.0f;
                npc->target_pos_y = (f32)(rand_u32((u32)i + 277) % 64) - 32.0f;
            }
        }
    }
}

void tick_engine(struct Engine* engine)
{
    struct GameState* prev_game_state = &engine->game_states[(engine->cur_game_state_idx + 1) & 1];
    struct GameState* next_game_state = &engine->game_states[engine->cur_game_state_idx];
    const s64 frame_num = engine->frame_num;
    
    struct GameInput game_input = {};
    ASSERT(prev_game_state->num_players == 32, "TODO: variable players");
    game_input.num_players = 32;
    ASSERT(game_input.num_players == 32, "TODO: variable players");
    platform_read_player_input(
        &game_input.player_input[0],
        prev_game_state->cam_pos_x,
        prev_game_state->cam_pos_y,
        prev_game_state->cam_w,
        prev_game_state->cam_aspect_ratio,
        prev_game_state->player_pos_x[0],
        prev_game_state->player_pos_y[0]);

    {
        const u32 num_players = prev_game_state->num_players;
        COPY(next_game_state->player_vel_x, prev_game_state->player_vel_x, num_players);
        COPY(next_game_state->player_vel_y, prev_game_state->player_vel_y, num_players);
        COPY(next_game_state->player_pos_x, prev_game_state->player_pos_x, num_players);
        COPY(next_game_state->player_pos_y, prev_game_state->player_pos_y, num_players);

        COPY(next_game_state->player_health, prev_game_state->player_health, num_players);
    }

    for(u64 i = 1; i < game_input.num_players; i++)
    {
        struct Npc* npc = &engine->npcs[i];
        update_npc(&game_input.player_input[i],
                npc,
                &engine->path_find,
                &LEVEL0,
                prev_game_state,
                (u32)i,
                frame_num);
    }

    // Player select NPCs.
    {
        ASSERT(game_input.num_players == 32, "TODO: variable players");
        const struct PlayerInput* player_input = &game_input.player_input[0];
        const v2 cursor_pos = make_v2(player_input->cursor_pos_x, player_input->cursor_pos_y);
    
        if(!player_input_get_bool(player_input, PLAYER_INPUT_SELECT))
        {
            engine->last_selected_player_id = 0;
        }
        for(u64 i = 0; i < prev_game_state->num_players; i++)
        {
            const v2 player_pos = make_v2(prev_game_state->player_pos_x[i], prev_game_state->player_pos_y[i]);
            const f32 player_radius = 0.5f;
            if(length_sq_v2(sub_v2(cursor_pos, player_pos)) <= sq_f32(player_radius))
            {
                engine->last_selected_player_id =
                    player_input_get_bool(player_input, PLAYER_INPUT_SELECT)
                    ? (u32)i
                    : engine->last_selected_player_id;
            }
        }
        if(player_input_get_bool(player_input, PLAYER_INPUT_SELECT))
        {
            struct Npc* npc = &engine->npcs[engine->last_selected_player_id];
            npc->target_pos_x = cursor_pos.x;
            npc->target_pos_y = cursor_pos.y;
        }
    }

#if 0   
    {
        struct PathFind* path_find = &engine->path_find;
        const s32 start_x = clamp_s32((s32)round_neg_inf(next_game_state->player_pos_x[0]), -64, 63);
        const s32 start_y = clamp_s32((s32)round_neg_inf(next_game_state->player_pos_y[0]), -32, 31);
        const s32 end_x = clamp_s32((s32)round_neg_inf(game_input.player_input[0].cursor_pos_x), -64, 63);
        const s32 end_y = clamp_s32((s32)round_neg_inf(game_input.player_input[0].cursor_pos_y), -32, 31);
        s32 path_x[MAX_PATH_LEN];
        s32 path_y[MAX_PATH_LEN];
        const u32 num_path = run_path_find(
            path_find,
            path_x,
            path_y,
            &LEVEL0,
            start_x,
            start_y,
            end_x,
            end_y);
        if(num_path)
        {
            for(u64 i = 0; i < num_path; i++)
            {
                debug_draw_add_world_quad(
                    (f32)path_x[i] + 0.5f,
                    (f32)path_y[i] + 0.5f,
                    0.5f,
                    1.0f,
                    1.0f,
                    0.0f,
                    1.0f,
                    0.0f,
                    1.0f);
            }
        }
    }
    #endif

    u8 bullet_is_dead[MAX_BULLETS] = {};
    {
        const u32 num_bullets = prev_game_state->num_bullets;
        COPY(next_game_state->bullet_vel_x, prev_game_state->bullet_vel_x, num_bullets);
        COPY(next_game_state->bullet_vel_y, prev_game_state->bullet_vel_y, num_bullets);
        COPY(next_game_state->bullet_pos_x, prev_game_state->bullet_pos_x, num_bullets);
        COPY(next_game_state->bullet_pos_y, prev_game_state->bullet_pos_y, num_bullets);
        COPY(next_game_state->bullet_team_id, prev_game_state->bullet_team_id, num_bullets);

        COPY(next_game_state->bullet_prev_pos_x, prev_game_state->bullet_pos_x, num_bullets);
        COPY(next_game_state->bullet_prev_pos_y, prev_game_state->bullet_pos_y, num_bullets);

        next_game_state->num_bullets = num_bullets;
        
        const struct PlayerInput* player_input = &game_input.player_input[0];
        if(player_input_get_bool(player_input, PLAYER_INPUT_SHOOT))
        {
            const v2 player_pos = make_v2(prev_game_state->player_pos_x[0], prev_game_state->player_pos_y[0]);
            const v2 cursor_pos = make_v2(player_input->cursor_pos_x, player_input->cursor_pos_y);

            v2 vel = normalize_or_v2(sub_v2(cursor_pos, player_pos), zero_v2());
            vel = scale_v2(vel, 400.0f);

            add_bullet(
                next_game_state,
                vel.x,
                vel.y,
                player_pos.x,
                player_pos.y,
                player_pos.x,
                player_pos.y,
                0);
        }
    }

    update_physics(next_game_state, bullet_is_dead, &game_input);

    {
        // Remove dead bullets.
        const u64 num_bullets = next_game_state->num_bullets;
        u64 i_dst = 0;
        for(u64 i_src = 0; i_src < num_bullets; i_src++)
        {
            if(!bullet_is_dead[i_src])
            {
                assign_bullet(
                    next_game_state,
                    i_dst,
                    next_game_state->bullet_vel_x[i_src],
                    next_game_state->bullet_vel_y[i_src],
                    next_game_state->bullet_pos_x[i_src],
                    next_game_state->bullet_pos_y[i_src],
                    next_game_state->bullet_prev_pos_x[i_src],
                    next_game_state->bullet_prev_pos_y[i_src],
                    next_game_state->bullet_team_id[i_src]
                );
                i_dst++;
            }
        }
        next_game_state->num_bullets = (u32)i_dst;
    }

    ASSERT(next_game_state->num_players > 0, "Must have at least 1 player");
    next_game_state->cam_pos_x = next_game_state->player_pos_x[0];
    next_game_state->cam_pos_y = next_game_state->player_pos_y[0];

    engine->cur_game_state_idx = (engine->cur_game_state_idx + 1) & 1;
    engine->frame_num++;
}
