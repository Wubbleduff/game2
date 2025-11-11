
#pragma once

#define MAX_LEVEL_WALLS 64
struct LevelWall
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
    struct LevelWall walls[MAX_LEVEL_WALLS];
};
