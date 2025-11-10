
#pragma once

#include "common.h"
#include "game_input.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

enum Win32MouseButton
{
    WIN32_MB_LEFT,
    WIN32_MB_RIGHT,
    WIN32_MB_MIDDLE,

    NUM_WIN32_MOUSE_BUTTONS
};

struct PlatformWin32Input
{
    // Bit array of virtual key codes.
    // https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    u8 keyboard_buf[0x100];
    u8 prev_keyboard_buf[0x100];

    u8 mouse_button[(NUM_WIN32_MOUSE_BUTTONS + 7) & ~7];
    u8 prev_mouse_button[(NUM_WIN32_MOUSE_BUTTONS + 7) & ~7];

    s32 mouse_screen_x;
    s32 mouse_screen_y;
    s32 prev_mouse_screen_x;
    s32 prev_mouse_screen_y;
};

struct PlatformWin32Input* platform_win32_get_input();

void platform_win32_init_input(struct PlatformWin32Input* mem);

// Indicate beginning of a frame for inputs. Used for rotating previous and next inputs.
void platform_win32_input_end_frame();

void platform_win32_set_keyboard_key(const u8 vk);
void platform_win32_clear_keyboard_key(const u8 vk);

void platform_win32_set_mouse_button(const enum Win32MouseButton m);
void platform_win32_clear_mouse_button(const enum Win32MouseButton m);

u32 platform_win32_is_keyboard_key_down(const u8 vk);
u32 platform_win32_is_keyboard_key_toggled_down(const u8 vk);
u32 platform_win32_is_mouse_button_down(const enum Win32MouseButton m);
u32 platform_win32_is_mouse_button_toggled_down(const enum Win32MouseButton m);

void platform_win32_get_mouse_screen_position(s32* x, s32* y);
void platform_win32_get_mouse_screen_delta(s32* x, s32* y);

////////////////////////////////////////////////////////////////////////////////
// Game specific impl

struct GameInput;
void platform_win32_read_game_input(
    struct GameInput* game_input,
    const f32 cam_pos_x,
    const f32 cam_pos_y,
    const f32 cam_width,
    const f32 cam_aspect_ratio);
