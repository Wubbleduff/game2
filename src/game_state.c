
#include "game_state.h"
#include "game_input.h"
#include "math.h"
#include "constants.h"
#include "platform.h"

#include "level0.h"

void init_game_state(struct GameState* game_state)
{
    game_state->cam_pos_x = 0.0f;
    game_state->cam_pos_y = 0.0f;
    game_state->cam_w = 60.0f;
    game_state->cam_aspect_ratio = get_screen_aspect_ratio();

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
            game_state->player_pos_z[num] = 0.5f;
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
            game_state->player_pos_z[num] = 0.5f;
            game_state->player_type[num] = 0;
            game_state->player_team_id[num] = 1;
            num++;
        }
        game_state->num_players = num;
    }
}

void update_game_state(
    struct GameState* game_state,
    const struct GameState* prev_game_state,
    const struct GameInput* game_input)
{
    ASSERT(prev_game_state->num_players == 32, "TODO: variable players");
    ASSERT(game_input->num_players == 32, "TODO: variable players");

    // Update physics.
    {
        const u32 num_iterations = 16;
        const f32 sub_dt = (f32)FRAME_DURATION_NS * (1.0f / 1000000000.0f) * (1.0f / (f32)num_iterations);
        const f32 max_accel = 4000.0f / (f32)num_iterations;
        const f32 drag = -250.0f / (f32)num_iterations;


        f32 player_vel_x[MAX_PLAYERS];
        f32 player_vel_y[MAX_PLAYERS];
        f32 player_pos_x[MAX_PLAYERS];
        f32 player_pos_y[MAX_PLAYERS];
        f32 player_radius[MAX_PLAYERS];
        COPY_ARRAY(player_vel_x, prev_game_state->player_vel_x);
        COPY_ARRAY(player_vel_y, prev_game_state->player_vel_y);
        COPY_ARRAY(player_pos_x, prev_game_state->player_pos_x);
        COPY_ARRAY(player_pos_y, prev_game_state->player_pos_y);
        const u32 num_players = prev_game_state->num_players;

        for(u32 i = 0; i < num_players; i++)
        {
            player_radius[i] = 0.5f;
        }

        // Iteratively update physics.
        for(u32 iteration = 0; iteration < num_iterations; iteration++)
        {
            // Note: The order with which we process kinematics and collisions is important.

            // 1. Integrate forces into player velocity.
            for(u32 i = 0; i < num_players; i++)
            {
                const struct PlayerInput* player_input = &game_input->player_input[i];
                const v2 move_dir = make_v2(player_input->player_move_x, player_input->player_move_y);
                v2 player_vel = make_v2(player_vel_x[i], player_vel_y[i]);

                v2 accel = zero_v2();

                accel = scale_v2(move_dir, max_accel);

                // Drag.
                // a' = a + v * d;
                accel = add_v2(accel, scale_v2(player_vel, drag));

                // v' = a*t + v
                player_vel = add_v2(player_vel, scale_v2(accel, sub_dt));

                player_vel_x[i] = player_vel.x;
                player_vel_y[i] = player_vel.y;
            }

            // 2. Resolve player-player collisions.
            for(u32 a_id = 0; a_id < num_players; a_id++)
            {
                const v2 a_pos = make_v2(player_pos_x[a_id], player_pos_y[a_id]);
                v2 a_vel = make_v2(player_vel_x[a_id], player_vel_y[a_id]);
                const f32 a_radius = player_radius[a_id];

                // Note: This technically makes us dependent on the order of updated players. We could fix this by introducing an intermediate buffer for
                //       position and velocity.
                for(u32 b_id = 0; b_id < num_players; b_id++)
                {
                    const v2 b_pos = make_v2(player_pos_x[b_id], player_pos_y[b_id]);
                    v2 b_vel = make_v2(player_vel_x[b_id], player_vel_y[b_id]);
                    const f32 b_radius = player_radius[b_id];

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

            // 3. Resolve player-wall collisions.
            //    Do this after all player-player collisions so it's harder for players to move into walls.
            for(u32 i = 0; i < num_players; i++)
            {
                const v2 player_pos = make_v2(player_pos_x[i], player_pos_y[i]);
                const f32 player_r = player_radius[i];

                v2 player_vel = make_v2(player_vel_x[i], player_vel_y[i]);

                ASSERT(prev_game_state->cur_level == 0, "TODO levels");
                const f32* wall_pos_x = level0_wall_pos_x;
                const f32* wall_pos_y = level0_wall_pos_y;
                const f32* wall_width = level0_wall_width;
                const f32* wall_height = level0_wall_height;
                const u32 num_walls = ARRAY_COUNT(level0_wall_pos_x);

                for(u32 i_wall = 0; i_wall < num_walls; i_wall++)
                {
                    const f32 wall_left = wall_pos_x[i_wall] - wall_width[i_wall] * 0.5f;
                    const f32 wall_right = wall_pos_x[i_wall] + wall_width[i_wall] * 0.5f;
                    const f32 wall_bottom = wall_pos_y[i_wall] - wall_height[i_wall] * 0.5f;
                    const f32 wall_top = wall_pos_y[i_wall] + wall_height[i_wall] * 0.5f;

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

                player_vel_x[i] = player_vel.x;
                player_vel_y[i] = player_vel.y;
            }

            // 4. Integrate velocity into position.
            for(u32 i = 0; i < num_players; i++)
            {
                const v2 player_vel = make_v2(player_vel_x[i], player_vel_y[i]);
                v2 player_pos = make_v2(player_pos_x[i], player_pos_y[i]);
                player_pos = add_v2(player_pos, scale_v2(player_vel, sub_dt));
                player_pos_x[i] = player_pos.x;
                player_pos_y[i] = player_pos.y;
            }
        }

        COPY_ARRAY(game_state->player_vel_x, player_vel_x);
        COPY_ARRAY(game_state->player_vel_y, player_vel_y);
        COPY_ARRAY(game_state->player_pos_x, player_pos_x);
        COPY_ARRAY(game_state->player_pos_y, player_pos_y);

        
    }

}
