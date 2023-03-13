#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <argp.h>
#include "go.h"
#include "commands.h"
#include "gtp.h"
struct flags { char* e_path; int size; };
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
    }
    return 0;
}

int main(int argc, char** argv)
{
    srand(time(NULL));
    Goban goban;
    ResetGoban(&goban);
    goban.color = 'b';

    struct flags flags;
    flags.e_path = NULL;
    flags.size = 19;
    
    struct argp_option options[] =
    {
        { "engine", 'e', "PATH", 0, "Supplies a go engine to play as White. To use gnugo you can use -e \"gnugo --mode gtp\""},
        { "size", 's', "NUM", 0, "Size of the goboard. Default is 19"},
        { 0 }
    };
    struct argp argp = { options, parse_opt };
    int r = argp_parse(&argp, argc, argv, 0, 0, &flags);
    goban.size = flags.size;

    Engine e;
    e.pid = -1;
    if (flags.e_path)
    {
        StartEngine(&e, flags.e_path);
        printf("%s %s\n", e.name, e.version);
        char** response = AllocateResponse();
        SendClearBoard(e.write, 1);
        int n_resp = GetResponse(e.read, response, 1);
        CleanResponse(response);
        FreeResponse(response);
    }
    int running = 1;
    while (running)
    {
        if (running == 2)
        {
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
        PrintBoard(&goban);
        if (e.pid >= 0 && goban.color == 'w') {
            int n_resp = 0;
            char** response = AllocateResponse();

            if (HistorySize() == 1)
            {
                SendClearBoard(e.write, 1);
                n_resp = GetResponse(e.read, response, 1);
                CleanResponse(response);
                SendBoardsize(e.write, 2, goban.size);
                n_resp = GetResponse(e.read, response, 2);
                CleanResponse(response);
            }

            SendPlay(e.write, 2, goban.lastmove, goban.size);
            n_resp = GetResponse(e.read, response, 2);
            CleanResponse(response);

            SendGenmove(e.write, 3, goban.color);
            n_resp = GetResponse(e.read, response, 3);
            printf("%s\n", response[0]);
            running = ProcessCommand(&goban, response[0]);

            CleanResponse(response);
            FreeResponse(response);
        }
        else{
            char input[COMMAND_LENGTH];
            if (!fgets(input, 256, stdin))
                exit(-1);
            running = ProcessCommand(&goban, input);
        }
    }
    return 0;
}
