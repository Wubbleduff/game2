#pragma once

#include "common.h"

f32 platform_get_screen_aspect_ratio();

struct PlayerInput;
void platform_read_player_input(
    struct PlayerInput* player_input,
    const f32 cam_pos_x,
    const f32 cam_pos_y,
    const f32 cam_width,
    const f32 cam_aspect_ratio,
    const f32 player_pos_x,
    const f32 player_pos_y);
