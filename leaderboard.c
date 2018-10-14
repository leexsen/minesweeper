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

/*
 * Find a record from the record list using player name
 * @params:
 *  game: the game object
 *  player_name: the name of the record to be searched
 */
static Record *record_get(char *player_name)
{
    for (RecordList *p = record_list_head; p != NULL; p = p->next) {
        if (strcmp(player_name, p->record->player_name) == 0)
            return p->record;
    }

    return NULL;
}

/*
 * Put a record object into the record list
 * @params:
 *  record: the record to be inserted
 */
static void record_put(Record *record)
{
    RecordList *p = (RecordList *)calloc(1, sizeof(RecordList));
    if (p == NULL)
        error_exit(ERROR_MEM_ALLOC);

    // the list is empty
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

/*
 * Compare two entries.
 * Return -1 if the first one is smaller than the second one.
 * Return 0 if the first one is equal to the second one.
 * Return 11 if the first one is larger than the second one.
 * @params:
 *  a: the first entry
 *  b: the second entry
 */
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

/*
 * Insert an entry into the leaderboard.
 * The entry in the head of the leaderboard is always greater than
 * the entry in the tail of the leaderboard
 * @params:
 *  node: the entry to be inserted
 */
static void leaderboard_list_put(LeaderboardList *node)
{
    if (leaderboard_list_head == NULL)
        leaderboard_list_head = node;

    else {

        // search the list to find a node that is smaller than the node to be inserted
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

/*
 * Update the leaderboard
 * @params:
 *  player_name: the name of the record to be searched
 *  duration: the duration of the game
 *  won: true if the user won
 */
void leaderboard_put(char *player_name, uint32_t duration, bool won)
{
    pthread_mutex_lock(&mutex);

    // get the historical record object of the user
    Record *record = record_get(player_name);

    // if this is first time the user played the game, create a record object
    // to store the historical info of the object.
    // Note. erver user only have one record object
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

        // link this entry to the historical record of the user
        node->entry.record = record;
        node->entry.duration = duration;

        leaderboard_list_put(node);
    }
    
    pthread_mutex_unlock(&mutex);
}

/*
 * Send the leaderboard clients
 * @params:
 *  socketfd: the socket connected with the client
 */
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

    // tells the client that the transmission of leaderboard is completed
    cmd.cmd_id = LEADERBOARD_END;
    send(socketfd, &cmd, sizeof(CmdBlock), 0);
    pthread_mutex_unlock(&mutex);
}


/*
 * Deallocate the record list and the leaderboard
 * @params:
 *  socketfd: the socket connected with the client
 */
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
