
#include "constants.h"
#include "game_state.h"
#include "path_find.h"
#include "math.h"

#include "platform_win32/platform_win32_core.h"
#include "platform_win32/platform_win32_input.h"
#include "platform_win32/platform_win32_render.h"

struct PlatformWin32
{
    struct PlatformWin32Core core;

    struct PlatformWin32Input input;

    struct PlatformWin32Render render;
};

struct MainMemory
{
    struct PlatformWin32 win32;

    s64 frame_num;

    u32 cur_game_state_idx;
    struct GameState game_states[2];

    struct PathFind path_find;
};
struct MainMemory* g_main_memory;

void platform_win32_init()
{
    platform_win32_init_core(&g_main_memory->win32.core);

    platform_win32_init_input(&g_main_memory->win32.input);

    platform_win32_init_render(&g_main_memory->win32.render);

    for(u64 i = 0; i < ARRAY_COUNT(g_main_memory->game_states); i++)
    {
        init_game_state(&g_main_memory->game_states[i]);
    }
    g_main_memory->cur_game_state_idx = 0;
}

void WinMainCRTStartup()
{
    g_main_memory = VirtualAlloc(
        NULL,
        (sizeof(struct MainMemory) + 4095) & ~4095,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE);
    ASSERT(g_main_memory, "Could not allocate memory.");
    ASSERT(((u64)g_main_memory & 4095) == 0, "g_main_memory not aligned.");
    memset(g_main_memory, 0xCD, sizeof(*g_main_memory));
    
    platform_win32_init();

    init_path_find(&g_main_memory->path_find);

    s64 frame_timer_ns = 0;
    s64 last_frame_time_ns = platform_win32_get_time_ns();

    s64 frame_num = 0;
    u64 running = 1;
    while(running)
    {
        const s64 frame_start_ns = platform_win32_get_time_ns();
        s64 frame_duration_ns = frame_start_ns - last_frame_time_ns;
        last_frame_time_ns = frame_start_ns;
        frame_timer_ns += frame_duration_ns;

        running &= platform_win32_read_events();

        if(frame_timer_ns < FRAME_DURATION_NS)
        {
            _mm_pause();
            continue;
        }
        frame_timer_ns -= FRAME_DURATION_NS;

        running &= !platform_win32_is_keyboard_key_down(VK_ESCAPE);

        struct GameState* game_state = &g_main_memory->game_states[g_main_memory->cur_game_state_idx];
        struct GameState* prev_game_state = &g_main_memory->game_states[(g_main_memory->cur_game_state_idx + 1) & 1];

        struct GameInput game_input = {};
        game_input.num_players = 32;
        platform_win32_read_player_input(
            &game_input.player_input[0],
            prev_game_state->cam_pos_x,
            prev_game_state->cam_pos_y,
            prev_game_state->cam_w,
            prev_game_state->cam_aspect_ratio);

        {
            struct PathFind* path_find = &g_main_memory->path_find;
            init_path_find(path_find);

            path_find_set_wall(path_find, 4, 4, 1, 1);

            path_find_set_wall(path_find, 1, 26, 12, 12);

            u8 start_x = (u8)(clamp_f32(game_state->player_pos_x[0] + 64.0f, 0.0f, 255.0f));
            u8 start_y = (u8)(clamp_f32(game_state->player_pos_y[0] + 32.0f, 0.0f, 255.0f));
            u8 end_x = (u8)(clamp_f32(game_input.player_input[0].cursor_pos_x + 64.0f, 0.0f, 255.0f));
            u8 end_y = (u8)(clamp_f32(game_input.player_input[0].cursor_pos_y + 32.0f, 0.0f, 255.0f));
            const u32 success = run_path_find(path_find, start_x, start_y, end_x, end_y);
            if(success)
            {
                u16 cur_idx = end_y * 256 + end_x;
                while(cur_idx != (start_y * 256 + start_x))
                {
                    const u8 x = (u8)(cur_idx & 0xFF);
                    const u8 y = (u8)(cur_idx >> 8);
                    add_world_quad(
                        (f32)x - 64.0f + 0.5f,
                        (f32)y - 32.0f + 0.5f,
                        0.5f,
                        1.0f,
                        1.0f,
                        0.0f,
                        1.0f,
                        0.0f,
                        1.0f);

                    cur_idx = path_find->grid_prev[cur_idx];
                }
            }
        }

        update_game_state(game_state, prev_game_state, &game_input);

        ASSERT(game_state->num_players > 0, "Must have at least 1 player");
        game_state->cam_pos_x = game_state->player_pos_x[0];
        game_state->cam_pos_y = game_state->player_pos_y[0];

        platform_win32_render(game_state);

        platform_win32_swap_and_clear_buffer(20, 0, 50);

        g_main_memory->cur_game_state_idx = (g_main_memory->cur_game_state_idx + 1) & 1;

        platform_win32_input_end_frame();
        
        frame_num++;
        g_main_memory->frame_num = frame_num;
    }

    ExitProcess(0);
}
