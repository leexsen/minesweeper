#ifndef __LEADERBOARD_H__
#define __LEADERBOARD_H__

#include <stdint.h>

typedef struct {
    char player_name[20];
    time_t duration;
    uint8_t num_won;
    uint8_t num_played;
} Leaderboard;

typedef struct leaderboard_list {
    Leaderboard leaderboard;
    struct leaderboard_list *next;
} LeaderboardList;

#endif