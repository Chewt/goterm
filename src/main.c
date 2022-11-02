#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "go.h"
#include "commands.h"
#include "gtp.h"


int main(int argc, char** argv)
{
    Engine e;
    e.pid = -1;
    if (argc > 1)
    {
        StartEngine(&e, argv[1]);
        printf("%s %s\n", e.name, e.version);
        char** response = AllocateResponse();
        SendClearBoard(e.write, 1);
        int n_resp = GetResponse(e.read, response, 1);
        CleanResponse(response);
        FreeResponse(response);
    }
    srand(time(NULL));
    Goban goban;
    ResetGoban(&goban);
    goban.size = 19;
    goban.color = 'b';
    int running = 1;
    while (running)
    {
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
                printf("here\n");
            }

            SendPlay(e.write, 2, goban.lastmove);
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
