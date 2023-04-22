#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <argp.h>
#include <poll.h>
#include "go.h"
#include "commands.h"
#include "gtp.h"
#include "sgf.h"
#include "networking.h"


struct flags {
  char *e_path;
  int size;
  int g;
  int is_server;
  char *host;
  int port;
  int swap;
};

const char *argp_program_bug_address = "<Hayden Johnson> hajohn100@gmail.com or at https://github.com/Chewt/goterm";
const char *argp_program_version = "version 1.0";

static int parse_opt (int key, char *arg, struct argp_state *state)
{
    struct flags *flags = state->input;
    switch (key)
    {
        case 'e': 
            flags->e_path = arg;
            break;
        case 's':
            flags->size = atoi(arg);
            break;
        case 'g':
            flags->g = 1;
            break;
        case 'h':
            flags->is_server = 1;
            break;
        case 'c':
            flags->host = arg;
            break;
        case 'p':
            flags->port = atoi(arg);
            break;
        case 500:
            flags->swap = 1;
            break;
    }
    return 0;
}

int main(int argc, char** argv)
{
    char gnugo[32] = "gnugo --mode gtp";
    char notes[NOTES_LENGTH];
    srand(time(NULL));
    Goban goban;
    goban.notes = notes;
    ResetGoban(&goban);
    goban.color = 'b';

    struct flags flags;
    flags.e_path = NULL;
    flags.size = 19;
    flags.g = 0;
    flags.is_server = 0;
    flags.host = NULL;
    flags.port = 5000;
    flags.swap = 0;
    
    struct argp_option options[] =
    {
        { "size", 's', "NUM", 0, "Size of the goboard. Default is 19."},
        {0,0,0,0, "Engines:", 7},
        { "engine", 'e', "PATH", 0, "Supplies a go engine to play as White. To use GNUgo you can use -e \"gnugo --mode gtp\"."},
        { "gnugo", 'g', 0, 0, "Play against GNUgo. Functionally identical to the example given for -e."},
        {0,0,0,0, "Networking:", 1},
        { "host", 'h', 0, 0, "Allow connections from another player"},
        { "connect", 'c', "IP", 0, "Connect to a host"},
        { "port", 'p', "PORT", 0, "Port for connecting to host"},
        { "swap", 500, 0, 0, "Swap colors for client and host (default host is white)"},
        { 0 }
    };
    struct argp argp = { options, parse_opt , 0, "Play Go/Baduk/Weiqi in the terminal!", 0, 0, 0};
    if (argp_parse(&argp, argc, argv, 0, 0, &flags))
    {
        fprintf(stderr, "Error parsing arguments\n");
        exit(-1);
    }
    goban.size = flags.size;

    if (flags.g == 1)
        flags.e_path = gnugo;

    Engine e;
    char e_col = 'w';
    e.pid = -1;
    if (flags.e_path)
    {
        StartEngine(&e, flags.e_path);
        printf("%s %s\n", e.name, e.version);
        char** response = AllocateResponse();
        SendClearBoard(e.write, 1);
        if (!GetResponse(e.read, response, 1))
            fprintf(stderr, "Couldn't get response from engine\n");
        FreeResponse(response);
    }

    int client = -1;
    int host = -1;
    char host_col = 'w';
    char client_col = 'b';
    if (flags.is_server && flags.port)
    {
        client = SetupServer(flags.port);
        if (flags.swap)
        {
            char t = host_col;
            host_col = client_col;
            client_col = t;
            SendCommand(client, "swap");
        }
        if (flags.size != 19)
        {
            char command[10] = "";
            snprintf(command, 9, "size %d", flags.size);
            SendCommand(client, command);
        }
        SendCommand(client, "ready");
    }
    else if (flags.host && flags.port)
    {
        host = SetupClient(flags.host, flags.port);
        int setup_finished = 0;
        while (!setup_finished)
        {
            char* resp = RecvCommand(host);
            printf("%s", resp);
            if (resp == NULL)
            {
                printf("Connection to other user broken\n");
                exit(0);
            }
            if (!strcmp(resp, "swap"))
            {
                char t = host_col;
                host_col = client_col;
                client_col = t;
            }
            else if (!strcmp(resp, "ready"))
            {
                printf("ready received\n");
                setup_finished = 1;
            }
            else
            {
                ProcessCommand(&goban, resp);
            }
            free(resp);
        }
    }

    int running = 1;
    while (running)
    {
        if (running == 2) /* Game is in complete state */
        {
            char resp[256];
            if (e.pid >= 0)
            {
                char** response = AllocateResponse();
                SendFinalScore(e.write, 1);
                if (!GetResponse(e.read, response, 1))
                    fprintf(stderr, "Couldn't get response from engine\n");
                strcpy(goban.result, response[1]);
                FreeResponse(response);
            }
            else
            {
                printf("Please enter a list of stones seperated by spaces\n"); 
                printf("(one stone per group)\n: ");
                if (fgets(resp, 256, stdin) == NULL)
                    continue;
                if (!RemoveDeadGroups(&goban, resp))
                {
                    PrintBoard(&goban);
                    continue;
                }
                ScoreBoard(&goban);
                PointDiff(&goban, resp);
            }
            if (host >= 0 || client >= 0)
            {
                printf("Confirming with opponent...\n");
                if (host >= 0) 
                {
                    SendCommand(host, resp);
                    char* confirm = RecvCommand(host);
                    if (!strcmp(confirm, "deny"))
                    {
                      snprintf(goban.notes, NOTES_LENGTH,
                               "Opponent disagrees with result, play on.\n");
                      UndoHistory(&goban);
                      free(confirm);
                      running = 1;
                      continue;
                    }
                    free(confirm);
                }
                else if (client >= 0)
                {
                    char* opponent_diff = RecvCommand(client);
                    if (strcmp(opponent_diff, resp))
                    {
                      snprintf(goban.notes, NOTES_LENGTH,
                               "Opponent disagrees with result, play on.\n");
                        SendCommand(client, "deny");
                        UndoHistory(&goban);
                        free(opponent_diff);
                        running = 1;
                        continue;
                    }
                    SendCommand(client, "confirm");
                    free(opponent_diff);
                }
            }
            AddHistory(&goban);
            char* sgf = CreateSGF(&e);
            if (sgf)
            {
                printf("%s\n\n", sgf);
                free(sgf);
            }
            printf("Result: %s\n", goban.result);
            printf("Game Over!\nPlay again?[y/N]");
            if (fgets(resp, 256, stdin) == NULL)
                break;
            if (resp[0] == 'y' || resp[0] == 'Y')
            {
                running = 1;
                ResetGoban(&goban);
                continue;
            }
            else 
                break;
        }
        PrintBoard(&goban);
        printf("%s", goban.notes);
        goban.notes[0] = '\0';
        if (e.pid >= 0 && goban.color == e_col) 
        {
            char** response = AllocateResponse();

            if (HistorySize() == 1)
            {
                SendClearBoard(e.write, 1);
                if (!GetResponse(e.read, response, 1))
                    fprintf(stderr, "Couldn't get response from engine\n");
                SendBoardsize(e.write, 2, goban.size);
                if (!GetResponse(e.read, response, 2))
                    fprintf(stderr, "Couldn't get response from engine\n");
                SendKomi(e.write, 3, goban.komi);
                if (!GetResponse(e.read, response, 3))
                    fprintf(stderr, "Couldn't get response from engine\n");
            }

            SendPlay(e.write, 2, goban.lastmove, goban.size);
            if (!GetResponse(e.read, response, 2))
                fprintf(stderr, "Couldn't get response from engine\n");

            SendGenmove(e.write, 3, goban.color);
            if (!GetResponse(e.read, response, 3))
                fprintf(stderr, "Couldn't get response from engine\n");
            printf("%s\n", response[1]);
            running = ProcessCommand(&goban, response[1]);
            if (running == MOVE)
                SubmitMove(&goban, response[1]);

            FreeResponse(response);
        } 
        else if (host >= 0 || client >= 0)
        {
            struct pollfd inputs[2];
            inputs[0].fd = 0;
            inputs[0].events = POLLIN;
            inputs[1].fd = (host >= 0) ? host : client;
            inputs[1].events = POLLIN;
            char opponent_color = (host >= 0) ? host_col : client_col;
            if (goban.color == opponent_color)
                printf("Waiting on opponent...\n");
            printf(": ");
            fflush(stdout);
            int ret_poll;
            while ((ret_poll = poll(inputs, 2, 100)) == 0);
            if (ret_poll > 0)
            {
                int user = (host >= 0) ? host : client;
                if (inputs[0].revents & POLLIN)
                {
                    char input[COMMAND_LENGTH];
                    if (!fgets(input, 256, stdin))
                        exit(-1);
                    input[strcspn(input, "\n")] = 0;

                    running = ProcessCommand(&goban, input);
                    if (running == MOVE && goban.color != opponent_color)
                        SubmitMove(&goban, input);
                    if (is_networked_command(input))
                        SendCommand(user, input);
                }
                if (inputs[1].revents & POLLIN)
                {
                    char* response = RecvCommand(user);
                    if (response == NULL)
                    {
                        printf("Connection to opponent broken\n");
                        break;
                    }
                    printf("%s\n", response);
                    running = ProcessCommand(&goban, response);
                    if (running == MOVE && goban.color == opponent_color)
                        SubmitMove(&goban, response);
                    free(response);
                }
            }
        }
        else
        {
            char input[COMMAND_LENGTH];
            printf(": ");
            if (!fgets(input, 256, stdin))
                exit(-1);
            input[strcspn(input, "\n")] = 0;

            running = ProcessCommand(&goban, input);
            if (running == MOVE)
                SubmitMove(&goban, input);
        }
        if (running == SWAP)
        {
            char t = host_col;
            host_col = client_col;
            client_col = t;
            e_col = (e_col == 'w') ? 'b' : 'w';
            running = 1;
        }
    }
    if (host >= 0)
        CloseClient(host);
    if (client >= 0)
        CloseServer(client);
    return 0;
}
