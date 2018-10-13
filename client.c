#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/signal.h>
#include <sys/socket.h>

#include "game.h"
#include "command.h"
#include "client_network.h"

static int32_t socketfd;

void print_welcome_msg(void)
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

void reveal_tile(CmdBlock *cmd, bool *game_over)
{
    while (true) {
        printf("Enter tile coordinates (e.g. A0): ");

        while (getchar() != '\n');
        scanf("%c%c", &(cmd->coord.x), &(cmd->coord.y));

        cmd->coord.x -= 'A';
        cmd->coord.y -= '0';

        cmd->cmd_id = REVEAL_TILE;
        send(socketfd, cmd, sizeof(CmdBlock), 0);
        recv(socketfd, cmd, sizeof(CmdBlock), 0);

        if (cmd->cmd_id == REVEAL_TILE_SUCCEED)
            break;

        else if (cmd->cmd_id == REVEAL_TILE_FAILED)
            puts("The coordinates could be either invalid or have been revealed, please try again");

        else if (cmd->cmd_id == LOSS) {
            *game_over = true;

            cmd->cmd_id = REQUEST_TILES_MINES;
            send(socketfd, cmd, sizeof(CmdBlock), 0);
            recv(socketfd, cmd, sizeof(CmdBlock), 0);

            printf("\n\nRemaining mines: %d\n\n", cmd->game_info.num_mines);
            print_tiles(cmd);
            puts("\n\n-------------------------");
            puts("Game over! You hit a mine");
            puts("-------------------------\n");
            break;
        }
    }
}

void place_flag(CmdBlock *cmd, bool *game_over)
{
    while (true) {
        printf("Enter tile coordinates (e.g. A0): ");

        while (getchar() != '\n');
        scanf("%c%c", &(cmd->coord.x), &(cmd->coord.y));

        cmd->coord.x -= 'A';
        cmd->coord.y -= '0';

        cmd->cmd_id = PLACE_FLAG;
        send(socketfd, cmd, sizeof(CmdBlock), 0);
        recv(socketfd, cmd, sizeof(CmdBlock), 0);

        if (cmd->cmd_id == PLACE_FLAG_SUCCEED)
            break;

        else if (cmd->cmd_id == PLACE_FLAG_FAILED)
            puts("The coordinates could be either invalid, no mines here or have been placed a flag, please try again");

        else if (cmd->cmd_id == WON) {
            *game_over = true;
            cmd->cmd_id = REQUEST_TILES_REVEALED;
            send(socketfd, cmd, sizeof(CmdBlock), 0);
            recv(socketfd, cmd, sizeof(CmdBlock), 0);

            printf("\n\nRemaining mines: %d\n\n", cmd->game_info.num_mines);
            print_tiles(cmd);
            puts("\n\n-----------------------------------------------");
            puts("Congratulations! You have located all the mines");
            puts("-----------------------------------------------");
            printf("You won in %d seconds!\n\n", ntohl(cmd->game_info.duration));
            break;
        }
    }
}

void print_leaderboard(CmdBlock *cmd)
{
    puts("\n------------------------------------------------------------------------------\n");

    recv(socketfd, cmd, sizeof(CmdBlock), 0);

    if (cmd->cmd_id == LEADERBOARD_END)
        puts("There is no information currently stored in the leaderboard. Try again later.");

    else {
        do {
            char *name = cmd->leaderboard.record.player_name;
            uint32_t duration = cmd->leaderboard.duration;
            uint8_t num_won = cmd->leaderboard.record.num_won;
            uint8_t num_played = cmd->leaderboard.record.num_played;

            printf("%s\t%10d seconds\t%5d games won, %5d games played\n", name, duration, num_won, num_played);
            recv(socketfd, cmd, sizeof(CmdBlock), 0);
            
        } while (cmd->cmd_id != LEADERBOARD_END);
    }

    puts("\n------------------------------------------------------------------------------\n\n");

}

void play_game(void)
{
    CmdBlock cmd;
    bool game_over = false;

    while (!game_over) {
        cmd.cmd_id = REQUEST_TILES_REVEALED;
        send(socketfd, &cmd, sizeof(CmdBlock), 0);
        recv(socketfd, &cmd, sizeof(CmdBlock), 0);

        printf("\n\nRemaining mines: %d\n\n", cmd.game_info.num_mines);

        print_tiles(&cmd);

        puts("\n\nChoose an option:");
        puts("<R> Reveal tile");
        puts("<P> Place flag");
        puts("<Q> Quit");
        printf("\nOption (R, P, Q): ");

        while (getchar() != '\n');
        char op = getchar();

        if (op == 'R') {
            reveal_tile(&cmd, &game_over);

        } else if (op == 'P') {
            place_flag(&cmd, &game_over);

        } else if (op == 'Q') {
            cmd.cmd_id = QUIT_GAME;
            send(socketfd, &cmd, sizeof(CmdBlock), 0);
            break;

        } else
            puts("Invalid inputs, please try again\n");
    }
}

void menu_selection(void)
{
    CmdBlock cmd;

    while (true) {
        puts("Please enter a selection:");
        puts("<1> Play Minesweeper");
        puts("<2> Show Leaderboard");
        puts("<3> Quit");
        printf("\nSelection option (1-3): ");

        while (getchar() != '\n');
        char op = getchar();

        if (op == '1') {
            cmd.cmd_id = START_GAME;
            send(socketfd, &cmd, sizeof(CmdBlock), 0);
            play_game();

        } else if (op == '2') {
            cmd.cmd_id = REQUEST_LEADERBOARD;
            send(socketfd, &cmd, sizeof(CmdBlock), 0);
            print_leaderboard(&cmd);

        } else if (op == '3') {
            cmd.cmd_id = DISCONNECT;
            send(socketfd, &cmd, sizeof(CmdBlock), 0);
            break;

        } else
            puts("Invalid inputs, please try again\n");
    }
}

void stop_client(int signal)
{
    CmdBlock cmd;

    cmd.cmd_id = DISCONNECT;
    send(socketfd, &cmd, sizeof(CmdBlock), 0);
    close(socketfd);
    exit(0);
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        puts("usage: client <server ip> <server port>");
        return 1;
    }

    signal(SIGINT, stop_client);
    print_welcome_msg();

    char *server_ip = argv[1];
    uint16_t port = atoi(argv[2]);
    socketfd = client_network_connect(server_ip, port);

    CmdBlock cmd;
    recv(socketfd, &cmd, sizeof(CmdBlock), 0);
        
    if (cmd.cmd_id == ASK_USERINFO) {

        get_user_info(cmd.user.name, cmd.user.password);
        cmd.cmd_id = USERINFO_RESPOND;
        send(socketfd, &cmd, sizeof(CmdBlock), 0);
        recv(socketfd, &cmd, sizeof(CmdBlock), 0);

        if (cmd.cmd_id == VALID_USER)
            menu_selection();
        else
            puts("\nYou entered either an incorrect username or password. Disconnecting.");
    }
    
    close(socketfd);
}
