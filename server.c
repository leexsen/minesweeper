#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <stdbool.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "socket_queue.h"
#include "thread_pool.h"
#include "leaderboard.h"
#include "server_network.h"
#include "authentication.h"
#include "command.h"
#include "game.h"

#define RANDOM_NUMBER_SEED 42
#define DEFAULT_PORT 12345
#define NUM_THREADS 10
#define QUEUE_CAPACITY 10

static volatile sig_atomic_t stop_running;
static int32_t server_socket;

void game_tiles_extract(Game *game, CmdBlock *cmd)
{
    uint8_t cmd_id = cmd->cmd_id;
    Tile (*game_tiles)[NUM_TILES_Y] = game->tiles;
    uint8_t (*cmd_tiles)[NUM_TILES_Y] = cmd->game_info.tiles;

    for (int i = 0; i < NUM_TILES_X; i++) {
        for (int j = 0; j < NUM_TILES_Y; j++) {
            if (game_tiles[i][j].is_mine) {

                // only send undiscovered mines according to the specification if game is over
                if (cmd_id == REQUEST_TILES_MINES && !game_tiles[i][j].revealed)
                    cmd_tiles[i][j] = '*';

                // send the positions of discovered mines
                else if (cmd_id == REQUEST_TILES_REVEALED && game_tiles[i][j].revealed)
                    cmd_tiles[i][j] = '+';

                else
                    cmd_tiles[i][j] = ' ';

            } else if (game_tiles[i][j].revealed) {

                if (cmd_id == REQUEST_TILES_REVEALED)
                    cmd_tiles[i][j] = game_tiles[i][j].adjacent_mines + '0';

                else
                    cmd_tiles[i][j] = ' ';

            } else
                cmd_tiles[i][j] = ' ';
        }
    }
}

void process_request(int32_t socketfd)
{
    Game *game;
    CmdBlock cmd;
    char player_name[20];

    cmd.cmd_id = ASK_USERINFO;
    send(socketfd, &cmd, sizeof(CmdBlock), 0);

    while (true) {
        recv(socketfd, &cmd, sizeof(CmdBlock), 0);

        if (cmd.cmd_id == USERINFO_RESPOND) {          
            if (authentication_isvalid(cmd.user.name, cmd.user.password)) {
                cmd.cmd_id = VALID_USER;
                send(socketfd, &cmd, sizeof(CmdBlock), 0);
                strncpy(player_name, cmd.user.name, 20);

            } else {
                cmd.cmd_id = INVALID_USER;
                send(socketfd, &cmd, sizeof(CmdBlock), 0);
                break;
            }

        } else if (cmd.cmd_id == START_GAME) {
            game = game_init(player_name);

        } else if (cmd.cmd_id == REVEAL_TILE) {
            int ret = game_reveal_tile(game, cmd.coord.x, cmd.coord.y);

            if (ret == 0)
                cmd.cmd_id = REVEAL_TILE_SUCCEED;
            else if (ret == -1)
                cmd.cmd_id = REVEAL_TILE_FAILED;
            else {
                cmd.cmd_id = LOSS;
                game_over(game, false);
            }

            send(socketfd, &cmd, sizeof(CmdBlock), 0);

        } else if (cmd.cmd_id == PLACE_FLAG) {
            int ret = game_place_flag(game, cmd.coord.x, cmd.coord.y);

            if (ret == 0)
                cmd.cmd_id = PLACE_FLAG_SUCCEED;
            else if (ret == -1)
                cmd.cmd_id = PLACE_FLAG_FAILED;
            else {
                cmd.cmd_id = WON;
                game_over(game, true);
            }

            send(socketfd, &cmd, sizeof(CmdBlock), 0);
 
        } else if (cmd.cmd_id == REQUEST_TILES_REVEALED) {
            game_tiles_extract(game, &cmd);
            cmd.cmd_id = TILES_REVEALD_RESPOND;
            cmd.game_info.num_mines = game->num_mines;
            cmd.game_info.duration = htonl(game->duration);
            send(socketfd, &cmd, sizeof(CmdBlock), 0);

        } else if (cmd.cmd_id == REQUEST_TILES_MINES) {
            game_tiles_extract(game, &cmd);
            cmd.cmd_id = TILES_MINES_RESPOND;
            cmd.game_info.num_mines = game->num_mines;
            send(socketfd, &cmd, sizeof(CmdBlock), 0);

        } else if (cmd.cmd_id == QUIT_GAME) {
            game_over(game, false);
            free(game);
            game = NULL;

        } else if (cmd.cmd_id == REQUEST_LEADERBOARD) {
            send_leaderboard(socketfd);

        } else if (cmd.cmd_id == DISCONNECT)
            break;
    }
}

void stop_server(int signal)
{
    stop_running = 1;
    close(server_socket);
}

void thread_cleanup_handler(void *arg)
{
    int32_t client_socket = *((int32_t *)arg);

    if (client_socket != -1)
        close(client_socket);

    else 
        socket_queue_cancellation();
}

void connection_handler(void)
{
    int32_t client_socket = -1;
    pthread_cleanup_push(thread_cleanup_handler, (void *)&client_socket);

    while (true) {
        client_socket = socket_queue_get();
        process_request(client_socket);
        close(client_socket);
        client_socket = -1;
    }

    pthread_cleanup_pop(0);
}

int main(int argc, char **argv)
{
    srand(RANDOM_NUMBER_SEED);
    signal(SIGINT, stop_server);

    authentication_init("Authentication.txt");
    socket_queue_init(QUEUE_CAPACITY);
    thread_pool_init(NUM_THREADS, connection_handler);

    uint16_t port = DEFAULT_PORT;
    if (argc > 1)
        port = atoi(argv[1]);
    
    server_socket = server_network_init(port);

    printf("Server: listening on port %d\n", port);

    while (!stop_running) {
        int32_t client_socket = server_network_accept(server_socket);
        if (client_socket != -1)
            socket_queue_put(client_socket);
    }

    thread_pool_cancel();
    thread_pool_join();
    thread_pool_destroy();

    socket_queue_destroy();
    leaderboard_destroy();
    authentication_destroy();

    return 0;
}
