#ifndef __GAME_H__
#define __GAME_H__

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#define NUM_TILES_X 9
#define NUM_TILES_Y 9
#define NUM_MINES 10

typedef struct {
    int8_t adjacent_mines;
    bool revealed;
    bool is_mine;
} Tile;

typedef struct {
    Tile tiles[NUM_TILES_X][NUM_TILES_Y];
    uint32_t duration;
    uint8_t num_mines;
    char player_name[20];
} Game;

extern Game *game_init(char *player_name);
extern int game_reveal_tile(Game *game, int8_t x, int8_t y);
extern int game_place_flag(Game *game, int8_t x, int8_t y);
extern void game_over(Game *game, bool won);

#endif
