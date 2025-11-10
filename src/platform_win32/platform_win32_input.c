
#include "math.h"
#include "game_input.h"

#include "platform_win32/platform_win32_input.h"
#include "platform_win32/platform_win32_core.h"

struct PlatformWin32Input* g_platform_win32_input;

struct PlatformWin32Input* platform_win32_get_input()
{
    return g_platform_win32_input;
}

void platform_win32_init_input(struct PlatformWin32Input* mem)
{
    struct PlatformWin32Core* win32_core = platform_win32_get_core();

    g_platform_win32_input = mem;

    struct PlatformWin32Input* win32_input = platform_win32_get_input();
    ZERO_ARRAY(win32_input->keyboard_buf);
    ZERO_ARRAY(win32_input->prev_keyboard_buf);

    POINT p;
    GetCursorPos(&p);
    ScreenToClient(win32_core->hwnd, &p);

    win32_input->mouse_screen_x = p.x;
    win32_input->mouse_screen_y = p.y;
    win32_input->prev_mouse_screen_x = p.x;
    win32_input->prev_mouse_screen_y = p.y;
}

void platform_win32_input_end_frame()
{
    struct PlatformWin32Input* win32_input = platform_win32_get_input();

    COPY_ARRAY(win32_input->prev_keyboard_buf, win32_input->keyboard_buf);
    COPY_ARRAY(win32_input->prev_mouse_button, win32_input->mouse_button);
}

static u8 is_valid_vk(const u8 k)
{
    return k >= 0x01 && k <= 0xFE;
}

void platform_win32_set_keyboard_key(const u8 vk)
{
    struct PlatformWin32Input* win32_input = platform_win32_get_input();
    ASSERT(is_valid_vk(vk), "Invalid keyboard key %u", vk);
    win32_input->keyboard_buf[vk >> 3] |= (1 << (vk & 7));
}

void platform_win32_clear_keyboard_key(const u8 vk)
{
    struct PlatformWin32Input* win32_input = platform_win32_get_input();
    ASSERT(is_valid_vk(vk), "Invalid keyboard key %u", vk);
    win32_input->keyboard_buf[vk >> 3] &= ~(1 << (vk & 7));
}

u32 platform_win32_is_keyboard_key_down(const u8 vk)
{
    struct PlatformWin32Input* win32_input = platform_win32_get_input();
    ASSERT(is_valid_vk(vk), "Invalid keyboard key %u", vk);
    return (win32_input->keyboard_buf[vk >> 3] & (1 << (vk & 7))) != 0;
}

u32 platform_win32_is_keyboard_key_toggled_down(const u8 vk)
{
    struct PlatformWin32Input* win32_input = platform_win32_get_input();
    ASSERT(is_valid_vk(vk), "Invalid keyboard key %u", vk);
    const u32 cur_frame = (win32_input->keyboard_buf[vk >> 3] & (1 << (vk & 7))) != 0;
    const u32 prev_frame = (win32_input->prev_keyboard_buf[vk >> 3] & (1 << (vk & 7))) != 0;
    return cur_frame && !prev_frame;
}

static u32 is_valid_mouse_button(const enum Win32MouseButton m)
{
    return m >= WIN32_MB_LEFT && m < NUM_WIN32_MOUSE_BUTTONS;
}

void platform_win32_set_mouse_button(const enum Win32MouseButton m)
{
    struct PlatformWin32Input* win32_input = platform_win32_get_input();
    ASSERT(is_valid_mouse_button(m), "Invalid mouse button %u", m);
    win32_input->mouse_button[m >> 3] |= (1 << (m & 7));
}

void platform_win32_clear_mouse_button(const enum Win32MouseButton m)
{
    struct PlatformWin32Input* win32_input = platform_win32_get_input();
    ASSERT(is_valid_mouse_button(m), "Invalid mouse button %u", m);
    win32_input->mouse_button[m >> 3] &= ~(1U << (m & 7));
}

u32 platform_win32_is_mouse_button_down(const enum Win32MouseButton m)
{
    struct PlatformWin32Input* win32_input = platform_win32_get_input();
    ASSERT(is_valid_mouse_button(m), "Invalid mouse button %u", m);
    return (win32_input->mouse_button[m >> 3] & (1U << (m & 7))) != 0;
}

u32 platform_win32_is_mouse_button_toggled_down(const enum Win32MouseButton m)
{
    struct PlatformWin32Input* win32_input = platform_win32_get_input();
    ASSERT(is_valid_mouse_button(m), "Invalid mouse button %u", m);
    const u32 cur_frame = (win32_input->mouse_button[m >> 3] & (1U << (m & 7))) != 0;
    const u32 prev_frame = (win32_input->prev_mouse_button[m >> 3] & (1U << (m & 7))) != 0;
    return cur_frame && !prev_frame;
}

void platform_win32_get_mouse_screen_position(s32* x, s32* y)
{
    struct PlatformWin32Input* win32_input = platform_win32_get_input();
    *x = win32_input->mouse_screen_x;
    *y = win32_input->mouse_screen_y;
}

void platform_win32_get_mouse_screen_delta(s32* x, s32* y)
{
    struct PlatformWin32Input* win32_input = platform_win32_get_input();
    *x = win32_input->mouse_screen_x - win32_input->prev_mouse_screen_x;
    *y = win32_input->mouse_screen_y - win32_input->prev_mouse_screen_y;
}


////////////////////////////////////////////////////////////////////////////////
// Game specific impl

void platform_win32_read_game_input(
    struct GameInput* game_input,
    const f32 cam_pos_x,
    const f32 cam_pos_y,
    const f32 cam_width,
    const f32 cam_aspect_ratio)
{
    const struct PlatformWin32Core* win32_core = platform_win32_get_core();
    const s32 screen_width = win32_core->client_width;
    const s32 screen_height = win32_core->client_height;

    s32 mouse_screen_x;
    s32 mouse_screen_y;
    platform_win32_get_mouse_screen_position(&mouse_screen_x, &mouse_screen_y);

    // Cursor world position.
    {
        const f32 cam_height = cam_width * cam_aspect_ratio;
        game_input->cursor_pos_x = (f32)cam_pos_x + ((f32)mouse_screen_x / (f32)screen_width - 0.5f) * cam_width;
        game_input->cursor_pos_y = (f32)cam_pos_y - ((f32)mouse_screen_y / (f32)screen_height - 0.5f) * cam_height;
    }
    
    // Player move.
    {
        v2 v = zero_v2();
        v.x += (f32)platform_win32_is_keyboard_key_down('D');
        v.x -= (f32)platform_win32_is_keyboard_key_down('A');
        v.y += (f32)platform_win32_is_keyboard_key_down('W');
        v.y -= (f32)platform_win32_is_keyboard_key_down('S');
        v = normalize_or_v2(v, zero_v2());
        game_input->player_move_x = v.x;
        game_input->player_move_y = v.y;
    }
}
