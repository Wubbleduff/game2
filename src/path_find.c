
#include "path_find.h"
#include "math.h"

void init_path_find(struct PathFind* path_find)
{
    ZERO_ARRAY(path_find->grid);
    path_find->num_open_list = 0;
    for(u64 i = 0; i < ARRAY_COUNT(path_find->grid_dist); i++)
    {
        path_find->grid_dist[i] = INFINITY;
    }
}

void path_find_set_wall(struct PathFind* path_find, u8 bl_x, u8 bl_y, u8 w, u8 h)
{
    for(u64 y = bl_y; y < bl_y + h; y++)
    {
        for(u64 x = bl_x; x < bl_x + w; x++)
        {
            const u64 idx = y * 256ULL + x;
            const u64 byte = idx / 8;
            const u64 bit = idx % 8;
            path_find->grid[byte] |= 1ULL << bit;
        }
    }
}

static u8 is_open_cell(const struct PathFind* path_find, const u8 x, const u8 y)
{
    const u64 idx = (u64)y * 256ULL + (u64)x;
    const u64 byte = idx / 8;
    const u64 bit = idx % 8;
    return !(path_find->grid[byte] & (1ULL << bit));
}

u32 run_path_find(
    struct PathFind* path_find,
    const u8 start_x,
    const u8 start_y,
    const u8 end_x,
    const u8 end_y)
{
    ASSERT(is_open_cell(path_find, start_x, start_y), "Invalid starting cell for path_find.");

    // For now, don't try to path find impossible paths.
    if(!is_open_cell(path_find, end_x, end_y))
    {
        return 0;
    }

    {
        const u8 cur_x = start_x;
        const u8 cur_y = start_y;
        const u16 cur_idx = (u16)((u64)cur_y * 256ULL + (u64)cur_x);
        path_find->num_open_list = 1;
        path_find->open_list[0] = cur_idx;
        path_find->grid_dist[cur_idx] = 0.0f;
        path_find->grid_prev[cur_idx] = cur_idx;
    }

    while(1)
    {
        u32 num_open_list = path_find->num_open_list;

        if(num_open_list == 0)
        {
            return 0;
        }

        u16 i_min = 0;
        f32 min_val = INFINITY;
        for(u64 i = 0; i < num_open_list; i++)
        {
            const u16 idx = path_find->open_list[i];
            const u8 x = (u8)((idx >> 0U) & 0xFF);
            const u8 y = (u8)((idx >> 8U) & 0xFF);
            const f32 hdist = sqrt_f32(sq_f32((f32)x - (f32)end_x) + sq_f32((f32)y - (f32)end_y));

            const f32 d = path_find->grid_dist[idx] + hdist;

            i_min = d < min_val ? (u16)i : i_min;
            min_val = d < min_val ? d : min_val;
        }
        ASSERT(min_val != INFINITY, "Bad val");
        ASSERT(i_min < num_open_list, "Bad val");

        const u16 cur_idx = path_find->open_list[i_min];
        const u8 cur_x = (u8)((cur_idx >> 0U) & 0xFF);
        const u8 cur_y = (u8)((cur_idx >> 8U) & 0xFF);
        const f32 cur_dist = path_find->grid_dist[(u64)cur_y * 256 + (u64)cur_x];

        {
            path_find->open_list[i_min] = path_find->open_list[num_open_list - 1];
            num_open_list--;
        }

        if(cur_x == end_x && cur_y == end_y)
        {
            return 1;
        }

        s64 dirs[8][2] =
        {
            { -1, -1 },
            {  0, -1 },
            {  1, -1 },
            { -1, 0 },
            {  1, 0 },
            { -1,  1 },
            {  0,  1 },
            {  1,  1 },
        };
        f32 dists[8] =
        {
            1.5f,
            1.0f,
            1.5f,
            1.0f,
            1.0f,
            1.5f,
            1.0f,
            1.5f,
        };

        for(u64 i_dir = 0; i_dir < 8; i_dir++)
        {
            const s64 n_x = (s64)cur_x + dirs[i_dir][0];
            const s64 n_y = (s64)cur_y + dirs[i_dir][1];
            const u16 n_idx = (u16)((u64)n_y * 256ULL + (u64)n_x);
            const f32 n_dist = cur_dist + dists[i_dir];
            const u32 add =
                n_x >= 0 &&
                n_x < 256 &&
                n_y >= 0 &&
                n_y < 256 &&
                is_open_cell(path_find, (u8)n_x, (u8)n_y) &&
                path_find->grid_dist[n_idx] > n_dist;
            path_find->grid_dist[n_idx] = add ? n_dist : path_find->grid_dist[n_idx];
            path_find->grid_prev[n_idx] = add ? cur_idx : path_find->grid_prev[n_idx];
            path_find->open_list[num_open_list] = n_idx;
            num_open_list += add;
        }

        path_find->num_open_list = num_open_list;
    }
}
