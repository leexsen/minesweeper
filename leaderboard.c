#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "error.h"
#include "command.h"
#include "leaderboard.h"

static RecordList *record_list_head;
static LeaderboardList *leaderboard_list_head;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static Record *record_get(char *player_name)
{
    for (RecordList *p = record_list_head; p != NULL; p = p->next) {
        if (strcmp(player_name, p->record->player_name) == 0)
            return p->record;
    }

    return NULL;
}

static void record_put(Record *record)
{
    RecordList *p = (RecordList *)calloc(1, sizeof(RecordList));
    if (p == NULL)
        error_exit(ERROR_MEM_ALLOC);

    if (record_list_head == NULL) {
        record_list_head = p;
        record_list_head->record = record;
    }

    else {
        p->record = record;
        p->next = record_list_head;
        record_list_head = p;
    }
}

static int compare(Entry *a, Entry *b)
{
    if (a->duration < b->duration)
        return -1;

    else if (a->duration > b->duration)
        return 1;

    else {
        if (a->record->num_won < b->record->num_won)
            return -1;

        else if (a->record->num_won > b->record->num_won)
            return 1;

        else
            return strcmp(a->record->player_name, b->record->player_name);
    }
}

static void leaderboard_list_put(LeaderboardList *node)
{
    if (leaderboard_list_head == NULL)
        leaderboard_list_head = node;

    else {
        for (LeaderboardList **p = &leaderboard_list_head ;; p = &((*p)->next)) {
            if (*p == NULL) {
                *p = node;
                break;
            }

            Entry *a = &(node->entry);
            Entry *b = &((*p)->entry);

            if (compare(a, b) >= 0) {
                node->next = *p;
                *p = node;
                break;
            }
        }
    }
}

void leaderboard_put(char *player_name, uint32_t duration, bool won)
{
    pthread_mutex_lock(&mutex);
    Record *record = record_get(player_name);

    if (record == NULL) {
        record = (Record *)calloc(1, sizeof(Record));
        if (record == NULL)
            error_exit(ERROR_MEM_ALLOC);

        strcpy(record->player_name, player_name);
        record_put(record);
    }

    (record->num_played)++;

    if (won) {
        (record->num_won)++;
        LeaderboardList *node = (LeaderboardList *)calloc(1, sizeof(LeaderboardList));
        if (node == NULL)
                error_exit(ERROR_MEM_ALLOC);

        node->entry.record = record;
        node->entry.duration = duration;

        leaderboard_list_put(node);
    }
    
    pthread_mutex_unlock(&mutex);
}

void send_leaderboard(int32_t socketfd)
{
    pthread_mutex_lock(&mutex);
    CmdBlock cmd;

    for (LeaderboardList *p = leaderboard_list_head; p != NULL; p = p->next) {
        
        cmd.cmd_id = LEADERBOARD_RECORD;
        cmd.leaderboard.duration = htonl(p->entry.duration);
        cmd.leaderboard.record.num_won = p->entry.record->num_won;
        cmd.leaderboard.record.num_played = p->entry.record->num_played;
        strcpy(cmd.leaderboard.record.player_name, p->entry.record->player_name);

        send(socketfd, &cmd, sizeof(CmdBlock), 0);
    }

    cmd.cmd_id = LEADERBOARD_END;
    send(socketfd, &cmd, sizeof(CmdBlock), 0);
    pthread_mutex_unlock(&mutex);
}

void leaderboard_destroy(void)
{
    for (RecordList *p = record_list_head; p != NULL; p = record_list_head) {
        record_list_head = p->next;
        free(p->record);
        free(p);
    }

    for (LeaderboardList *p = leaderboard_list_head; p != NULL; p = leaderboard_list_head) {
        leaderboard_list_head = p->next;
        free(p);
    }
}
