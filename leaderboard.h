#ifndef __LEADERBOARD_H__
#define __LEADERBOARD_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char player_name[20];
    uint8_t num_won;
    uint8_t num_played;
} Record;

typedef struct record_list{
    Record *record;
    struct record_list *next;
} RecordList;

typedef struct {
    Record *record;
    uint32_t duration;
} Entry;

typedef struct leaderboard_list {
    Entry entry;
    struct leaderboard_list *next;
} LeaderboardList;

extern void leaderboard_put(char *player_name, uint32_t duration, bool won);
extern void send_leaderboard(int32_t socketfd);
extern void leaderboard_destroy(void);

#endif
