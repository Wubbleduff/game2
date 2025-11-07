
#include "constants.h"
#include "game_state.h"

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

        update_game_state(game_state, prev_game_state);

        platform_win32_render(game_state);

        platform_win32_swap_and_clear_buffer(10, 150, 230);

        g_main_memory->cur_game_state_idx = (g_main_memory->cur_game_state_idx + 1) & 1;

        platform_win32_input_end_frame();
        
        frame_num++;
        g_main_memory->frame_num = frame_num;
    }

    ExitProcess(0);
}
