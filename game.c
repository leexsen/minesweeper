#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

#include "error.h"
#include "game.h"
#include "leaderboard.h"

static pthread_mutex_t mutex_rand = PTHREAD_MUTEX_INITIALIZER;

static void update_adjacent_tiles(int x, int y, Tile (*tiles)[NUM_TILES_Y])
{
    for (int i = -1; i <= 1; i++) {
        int new_x = x + i;

        if (new_x < 0 || new_x >= NUM_TILES_X)
            continue;

        for (int j = -1; j <= 1; j++) {
            int new_y = y + j;

            if (new_y < 0 || new_y >= NUM_TILES_Y || tiles[new_x][new_y].is_mine)
                continue;

            (tiles[new_x][new_y].adjacent_mines)++;
        }
    }
}

static void place_mines(Game *game)
{
    int x, y;
    uint8_t num_mines = game->num_mines;
    Tile (*tiles)[NUM_TILES_Y] = game->tiles;

    pthread_mutex_lock(&mutex_rand);
    for (uint8_t i = 0; i < num_mines; i++) {
        do {
            x = rand() % NUM_TILES_X;
            y = rand() % NUM_TILES_Y;
        } while (tiles[x][y].is_mine || tiles[x][y].adjacent_mines != 0);

        tiles[x][y].is_mine = true;
        update_adjacent_tiles(x, y, tiles);
    }
    pthread_mutex_unlock(&mutex_rand);
}

Game *game_init(char *player_name)
{
    Game *game = (Game *)calloc(1, sizeof(Game));
    if (game == NULL)
        error_exit(ERROR_MEM_ALLOC);

    game->num_mines = NUM_MINES;
    strncpy(game->player_name, player_name, 20);
    place_mines(game);
    game->duration = time(NULL);

    return game;
}

int game_reveal_tile(Game *game, int8_t x, int8_t y)
{
    Tile (*tiles)[NUM_TILES_Y] = game->tiles;

    if ((x < 0 || x >= NUM_TILES_X) || 
        (y < 0 || y >= NUM_TILES_Y))
        return -1; // reveal tile failed

    if (tiles[x][y].revealed)
        return -1; // reveal tile failed
        
    if (tiles[x][y].is_mine)
        return -2; // loss

    tiles[x][y].revealed = true;

    if (tiles[x][y].adjacent_mines == 0) {
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++)
                game_reveal_tile(game, x+i, y+j);
        }
    }
    
    return 0; // reveal tile successful
}

int game_place_flag(Game *game, int8_t x, int8_t y)
{
    Tile (*tiles)[NUM_TILES_Y] = game->tiles;

    if ((x < 0 || x >= NUM_TILES_X) || 
        (y < 0 || y >= NUM_TILES_Y))
        return -1; // place flge failed
        
    if (!tiles[x][y].is_mine || tiles[x][y].revealed)
        return -1; // place flge failed

    tiles[x][y].revealed = true;

    if (--(game->num_mines) == 0)
        return 1; // win
    
    return 0; // place flag successful
}

void game_over(Game *game, bool won)
{
    game->duration = time(NULL) - game->duration;
    leaderboard_put(game->player_name, game->duration, won);
}
