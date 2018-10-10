#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/socket.h>

#include "game.h"
#include "command.h"
#include "client_network.h"

void print_welcom_msg(void)
{
    puts("-----------------------------------------------");
    puts("Welcome to the online Minesweeper gaming system");
    puts("-----------------------------------------------\n");
    puts("Connecting to the server\n");
}

void get_user_info(char *name, char *password)
{
    puts("You are required to log on with your registered user name and password.\n");
    printf("Username: ");
    scanf("%19s", name);
    printf("Password: ");
    scanf("%19s", password);
    putchar('\n');
}

void print_tiles(CmdBlock *cmd)
{
    uint8_t (*tiles)[NUM_TILES_Y] = cmd->game_info.tiles;

    puts("    0 1 2 3 4 5 6 7 8");
    puts("---------------------");

    for (int i = 0; i < NUM_TILES_X; i++) {
        printf("%c | ", 'A' + i);

        for (int j = 0; j < NUM_TILES_Y; j++) {
            printf("%c ", tiles[i][j]);
        }

        putchar('\n');
    }
}

void reveal_tile(int32_t socketfd, CmdBlock *cmd, bool *game_over)
{
    while (true) {
        printf("Enter tile coordinates: ");

        getchar();
        scanf("%c%c", &(cmd->coord.x), &(cmd->coord.y));
        cmd->coord.x -= 'A';
        cmd->coord.y -= '0';

        cmd->cmd_id = REVEAL_TILE;
        send(socketfd, cmd, sizeof(CmdBlock), 0);
        recv(socketfd, cmd, sizeof(CmdBlock), 0);

        if (cmd->cmd_id == REVEAL_TILE_SUCCEED)
            break;
        else if (cmd->cmd_id == REVEAL_TILE_FAILED)
            puts("Invalied coordinates, please try again");
        else if (cmd->cmd_id == LOSS) {
            *game_over = true;
            cmd->cmd_id = REQUEST_TILES_MINES;
            send(socketfd, cmd, sizeof(CmdBlock), 0);
            recv(socketfd, cmd, sizeof(CmdBlock), 0);

            printf("\n\nRemaining mines: %d\n\n", cmd->game_info.num_mines);
            print_tiles(cmd);

            puts("\n\nGame over! You hit a mine\n");
            break;
        }
    }
}

void place_flag(int32_t socketfd, CmdBlock *cmd, bool *game_over)
{
    while (true) {
        printf("Enter tile coordinates: ");

        getchar();
        scanf("%c%c", &(cmd->coord.x), &(cmd->coord.y));
        cmd->coord.x -= 'A';
        cmd->coord.y -= '0';

        cmd->cmd_id = PLACE_FLAG;
        send(socketfd, cmd, sizeof(CmdBlock), 0);
        recv(socketfd, cmd, sizeof(CmdBlock), 0);

        if (cmd->cmd_id == PLACE_FLAG_SUCCEED)
            break;
        else if (cmd->cmd_id == PLACE_FLAG_FAILED)
            puts("Invalied coordinates, please try again");
        else if (cmd->cmd_id == WON) {
            *game_over = true;
            cmd->cmd_id = REQUEST_TILES_ALL;
            send(socketfd, cmd, sizeof(CmdBlock), 0);
            recv(socketfd, cmd, sizeof(CmdBlock), 0);

            printf("\n\nRemaining mines: %d\n\n", cmd->game_info.num_mines);
            print_tiles(cmd);

            puts("\n\nCongratulations! You have located all the mines\n");
            break;
        }
    }
}

void play_game(int32_t socketfd)
{
    CmdBlock cmd;
    bool game_over = false;

    while (!game_over) {
        cmd.cmd_id = REQUEST_TILES_ALL;
        send(socketfd, &cmd, sizeof(CmdBlock), 0);
        recv(socketfd, &cmd, sizeof(CmdBlock), 0);

        printf("\n\nRemaining mines: %d\n\n", cmd.game_info.num_mines);

        print_tiles(&cmd);

        puts("\n\nChoose an option:");
        puts("<R> Reveal tile");
        puts("<P> Place flag");
        puts("<Q> Quit");
        printf("\nOption (R, P, Q): ");

        char op;
        do { op = getchar(); } while (op == ' ' || op == '\n');

        if (op == 'R') {
            reveal_tile(socketfd, &cmd, &game_over);

        } else if (op == 'P') {
            place_flag(socketfd, &cmd, &game_over);
            //
        } else if (op == 'Q') {
            cmd.cmd_id = QUIT_GAME;
            send(socketfd, &cmd, sizeof(CmdBlock), 0);
            break;
        } else
            puts("Invalid inputs, please try again\n");
    }
}

void menu_selection(int32_t socketfd)
{
    CmdBlock cmd;

    while (true) {
        puts("Please enter a selection:");
        puts("<1> Play Minesweeper");
        puts("<2> Show Leaderboard");
        puts("<3> Quit");
        printf("\nSelection option (1-3): ");

        char op;
        do { op = getchar(); } while (op == ' ' || op == '\n');

        if (op == '1') {
            cmd.cmd_id = START_GAME;
            send(socketfd, &cmd, sizeof(CmdBlock), 0);
            play_game(socketfd);

        } else if (op == '2') {
            cmd.cmd_id = REQUEST_LEADERBOARD;
            send(socketfd, &cmd, sizeof(CmdBlock), 0);
            //
        } else if (op == '3') {
            cmd.cmd_id = DISCONNECT;
            send(socketfd, &cmd, sizeof(CmdBlock), 0);
            break;
        } else
            puts("Invalid inputs, please try again\n");
    }
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        puts("usage: client <server ip> <server port>");
        return 1;
    }

    print_welcom_msg();

    char *server_ip = argv[1];
    uint16_t port = atoi(argv[2]);
    int32_t socketfd = client_network_connect(server_ip, port);

    CmdBlock cmd;
    recv(socketfd, &cmd, sizeof(CmdBlock), 0);
        
    if (cmd.cmd_id == ASK_USERINFO) {
        get_user_info(cmd.user_info.name, cmd.user_info.password);
        cmd.cmd_id = USERINFO_RESPOND;
        send(socketfd, &cmd, sizeof(CmdBlock), 0);
        recv(socketfd, &cmd, sizeof(CmdBlock), 0);

        if (cmd.cmd_id == VALID_USER)
            menu_selection(socketfd);
        else
            puts("\nYou entered either an incorrect username or password. Disconnecting.");
    }
    
    close(socketfd);
}