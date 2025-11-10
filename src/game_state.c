
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

    game_state->player_vel_x = 0.0f;
    game_state->player_vel_y = 0.0f;
    game_state->player_pos_x = 0.0f;
    game_state->player_pos_y = 0.0f;
    game_state->player_pos_z = 0.5f;

    game_state->player_radius = 0.5f;

    game_state->cur_level = 0;
}

void update_game_state(
    struct GameState* game_state,
    const struct GameState* prev_game_state,
    const struct GameInput* game_input)
{
    // Update physics.
    {
        const u32 num_iterations = 16;
        const f32 sub_dt = (f32)FRAME_DURATION_NS * (1.0f / 1000000000.0f) * (1.0f / (f32)num_iterations);
        const f32 max_accel = 4000.0f / (f32)num_iterations;
        const f32 drag = -250.0f / (f32)num_iterations;

        const v2 move_dir = make_v2(game_input->player_move_x, game_input->player_move_y);

        // Iteratively update physics.
        v2 player_vel = make_v2(prev_game_state->player_vel_x, prev_game_state->player_vel_y);
        v2 player_pos = make_v2(prev_game_state->player_pos_x, prev_game_state->player_pos_y);
        for(u32 iteration = 0; iteration < num_iterations; iteration++)
        {
            // Note: The order with which we process kinematics and collisions is important.

            // 1. Integrate forces into player velocity.
            {
                v2 accel = scale_v2(move_dir, max_accel);

                // Drag.
                // a' = a + v * d;
                accel = add_v2(accel, scale_v2(player_vel, drag));

                // v' = a*t + v
                player_vel = add_v2(player_vel, scale_v2(accel, sub_dt));
            }

            // 5. Resolve player-wall collisions.
            //    Do this after all player-player collisions so it's harder for players to move into walls.
            {
                ASSERT(prev_game_state->cur_level == 0, "TODO levels");
                const f32 player_radius = game_state->player_radius;

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
                    if(dot_v2(n, n) < sq_f32(player_radius) &&
                        dot_v2(n, player_vel) < 0.0f)
                    {
                        const f32 j = dot_v2(scale_v2(player_vel, -1.0f), n) / dot_v2(n, n);
                        player_vel = add_v2(player_vel, scale_v2(n, j));
                    }
                }
            }

            // 6. Integrate velocity into position.
            {
                player_pos = add_v2(player_pos, scale_v2(player_vel, sub_dt));
            }
        }

        game_state->player_vel_x = player_vel.x;
        game_state->player_vel_y = player_vel.y;

        game_state->player_pos_x = player_pos.x;
        game_state->player_pos_y = player_pos.y;

        game_state->cam_pos_x = player_pos.x;
        game_state->cam_pos_y = player_pos.y;
    }

}
