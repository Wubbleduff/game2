
#pragma once

#include "common.h"
#include "constants.h"

enum PlayerInputBool
{
    PLAYER_INPUT_SELECT,
    PLAYER_INPUT_SHOOT,

    NUM_PLAYER_INPUT_BOOL
};

struct PlayerInput
{
    f32 move_x;
    f32 move_y;

    f32 cursor_pos_x;
    f32 cursor_pos_y;

    u8 bools[(NUM_PLAYER_INPUT_BOOL + 7) / 8];
};

struct GameInput
{
    u32 num_players;
    struct PlayerInput player_input[MAX_PLAYERS];
};

static inline u8 player_input_get_bool(const struct PlayerInput* in, const enum PlayerInputBool b)
{
    ASSERT(b < NUM_PLAYER_INPUT_BOOL, "Invalid player input bool %u", (u32)b);
    const u64 bit = (u64)b % 8;
    const u64 byte = (u64)b / 8;
    return (in->bools[byte] & (1ULL << bit)) != 0;
}

static inline void player_input_assign_bool(struct PlayerInput* in, const enum PlayerInputBool b, const u8 val)
{
    ASSERT(b < NUM_PLAYER_INPUT_BOOL, "Invalid player input bool %u", (u32)b);
    const u64 bit = (u64)b % 8;
    const u64 byte = (u64)b / 8;
    in->bools[byte] = 
        val
        ? (in->bools[byte] |  (1ULL << bit))
        : (in->bools[byte] & ~(1ULL << bit));
}
