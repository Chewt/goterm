#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <argp.h>
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
    char notes[256];
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
    flags.port = 0;
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
    e.pid = -1;
    if (flags.e_path)
    {
        StartEngine(&e, flags.e_path);
        printf("%s %s\n", e.name, e.version);
        char** response = AllocateResponse();
        SendClearBoard(e.write, 1);
        if (!GetResponse(e.read, response, 1))
            fprintf(stderr, "Couldn't get response from engine\n");
        CleanResponse(response);
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
        SendCommand(client, "ready");
    }
    else if (flags.host && flags.port)
    {
        host = SetupClient(flags.host, flags.port);
        int setup_finished = 0;
        while (!setup_finished)
        {
            char* resp = RecvCommand(host);
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
            if (e.pid >= 0)
            {
                char** response = AllocateResponse();
                SendFinalScore(e.write, 1);
                if (!GetResponse(e.read, response, 1))
                    fprintf(stderr, "Couldn't get response from engine\n");
                printf("Result: %s\n", response[1]);
                CleanResponse(response);
                FreeResponse(response);
            }
            char* sgf = CreateSGF(&e);
            if (sgf)
            {
                printf("%s\n", sgf);
                free(sgf);
            }
            char resp[256];
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
        printf("\e[2J\e[H");
        PrintBoard(&goban);
        printf("%s", goban.notes);
        goban.notes[0] = '\0';
        if (e.pid >= 0 && goban.color == 'w') 
        {
            char** response = AllocateResponse();

            if (HistorySize() == 1)
            {
                SendClearBoard(e.write, 1);
                if (!GetResponse(e.read, response, 1))
                    fprintf(stderr, "Couldn't get response from engine\n");
                CleanResponse(response);
                SendBoardsize(e.write, 2, goban.size);
                if (!GetResponse(e.read, response, 2))
                    fprintf(stderr, "Couldn't get response from engine\n");
                CleanResponse(response);
            }

            SendPlay(e.write, 2, goban.lastmove, goban.size);
            if (!GetResponse(e.read, response, 2))
                fprintf(stderr, "Couldn't get response from engine\n");
            CleanResponse(response);

            SendGenmove(e.write, 3, goban.color);
            if (!GetResponse(e.read, response, 3))
                fprintf(stderr, "Couldn't get response from engine\n");
            printf("%s\n", response[1]);
            running = ProcessCommand(&goban, response[1]);

            CleanResponse(response);
            FreeResponse(response);
        }
        else if (host >= 0 && goban.color == host_col)
        {
            printf("Waiting on opponent...\n");
            char* response = RecvCommand(host);
            if (response == NULL)
            {
                printf("Connection to host broken\n");
                break;
            }
            printf("%s\n", response);
            running = ProcessCommand(&goban, response);
            free(response);
        }
        else if (client >= 0 && goban.color == client_col)
        {
            printf("Waiting on opponent...\n");
            char* response = RecvCommand(client);
            if (response == NULL)
            {
                printf("Connection to client broken\n");
                break;
            }
            printf("%s\n", response);
            running = ProcessCommand(&goban, response);
            free(response);
        }
        else{
            char input[COMMAND_LENGTH];
            printf(": ");
            if (!fgets(input, 256, stdin))
                exit(-1);

            if (host >= 0 && is_networked_command(input))
                SendCommand(host, input);
            else if (client >= 0 && is_networked_command(input))
                SendCommand(client, input);
            running = ProcessCommand(&goban, input);
        }
    }
    if (host >= 0)
        CloseClient(host);
    if (client >= 0)
        CloseServer(client);
    return 0;
}
