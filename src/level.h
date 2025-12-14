
#pragma once

#define MAX_LEVEL_WALLS 64
struct LevelWallGeometry
{
    // Origin is the bottom left.
    s32 x;
    s32 y;
    u32 w;
    u32 h;
};
struct Level
{
    u32 width;
    u32 height;

    u32 num_walls;
    struct LevelWallGeometry walls[MAX_LEVEL_WALLS];
    f32 wall_color_bg[MAX_LEVEL_WALLS][3];
    f32 wall_color_hl[MAX_LEVEL_WALLS][3];

    u32 num_flags;
    f32 flag_pos_x[2];
    f32 flag_pos_y[2];
};
