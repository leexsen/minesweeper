#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <stdint.h>
#include <time.h>

#include "game.h"
#include "Leaderboard.h"
#include "authentication.h"

enum command {
    ASK_USERINFO,
    USERINFO_RESPOND,

    VALID_USER,
    INVALID_USER,

    START_GAME,
    
    REQUEST_TILES_MINES,
    REQUEST_TILES_REVEALED,

    TILES_MINES_RESPOND,
    TILES_REVEALD_RESPOND,

    REVEAL_TILE,
    REVEAL_TILE_SUCCEED,
    REVEAL_TILE_FAILED,
    LOSS,

    PLACE_FLAG,
    PLACE_FLAG_SUCCEED,
    PLACE_FLAG_FAILED,
    WON,
    
    REQUEST_LEADERBOARD,
    LEADERBOARD_RECORD,
    LEADERBOARD_EMPTY,

    QUIT_GAME,
    DISCONNECT
};

typedef struct {
    uint8_t cmd_id;

    union {
        struct {
            int8_t x, y;
        } coord;

        struct {
            uint8_t tiles[NUM_TILES_X][NUM_TILES_Y];
            uint8_t num_mines;
            time_t duration;
        } game_info;

        User user;
        Leaderboard Leaderboard;
    };
} CmdBlock;

#endif