
#pragma once

#include "common.h"

struct PathFind
{
    // 256x256 grid pathfind

    // Bitarray of valid cells.
    u8 grid[256 * 256 / 8];
    f32 grid_dist[256 * 256];
    u16 grid_prev[256 * 256];

    u32 num_open_list;
    u16 open_list[65536];
};

void init_path_find(struct PathFind* path_find);

void path_find_set_wall(struct PathFind* path_find, u8 x, u8 y, u8 w, u8 h);

u32 run_path_find(
    struct PathFind* path_find,
    const u8 start_x,
    const u8 start_y,
    const u8 end_x,
    const u8 end_y);


