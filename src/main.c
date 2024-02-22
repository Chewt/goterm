#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <argp.h>
#include <poll.h>
#include <ncurses.h>
#include <locale.h>
#include "gameinfo.h"
#include "gametree.h"
#include "go.h"
#include "commands.h"
#include "gtp.h"
#include "sgf.h"
#include "networking.h"
#include "display.h"


// argp struct
struct flags {
  char *e_path;
  int size;
  int g;
  int is_server;
  char *host;
  int port;
  int swap;
  char* sgf;
  int handicap;
};

const char *argp_program_bug_address = "Hayden Johnson <hajohn100@gmail.com> or at https://github.com/Chewt/goterm";
#ifdef VERSION
const char *argp_program_version = VERSION;
#endif

// Parsing args
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
        case 'c':
            flags->host = arg;
            break;
        case 'p':
            flags->port = atoi(arg);
            break;
        case 'f':
            flags->sgf = ReadSGFFile(arg);
            break;
        case 'h':
            flags->handicap = strtod(arg, NULL);
            break;
        case 500:
            flags->swap = 1;
            break;
        case 501:
            flags->is_server = 1;
            break;
    }
    return 0;
}

int main(int argc, char** argv)
{
    // Default args
    struct flags flags;
    flags.e_path = NULL;
    flags.size = 19;
    flags.g = 0;
    flags.is_server = 0;
    flags.host = NULL;
    flags.port = 5000;
    flags.swap = 0;
    flags.sgf = NULL;
    flags.handicap = 0;
    
    // gnugo shortcut
    char gnugo[32] = "gnugo --mode gtp";

    // Board Setup
    srand(time(NULL));
    GameInfo* gameInfo = GetGameInfo();
    Goban goban = {0};
    ResetGoban(&goban);
    goban.color = 'b';
    strcpy(gameInfo->blackName, "Black");
    strcpy(gameInfo->whiteName, "White");

    // argp parsing args
    struct argp_option options[] =
    {
        { "size", 's', "NUM", 0, "Size of the goboard. Default is 19."},
        { "handicap", 'h', "NUM", 0, "An optional handicap to start the game with."},
        { "sgf", 'f', "FILE", 0, "An optional SGF file to load"},
        {0,0,0,0, "Engines:", 7},
        { "engine", 'e', "PATH", 0, "Supplies a go engine to play as White. To use GNUgo you can use -e \"gnugo --mode gtp\"."},
        { "gnugo", 'g', 0, 0, "Play against GNUgo. Functionally identical to the example given for -e."},
        {0,0,0,0, "Networking:", 1},
        { "host", 501, 0, 0, "Allow connections from another player"},
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
    gameInfo->boardSize = flags.size;
    SetHandicap(&goban, flags.handicap);
    if (flags.handicap)
        goban.color = 'w';

    // apply gnugo shortcut
    if (flags.g == 1)
        flags.e_path = gnugo;

    // Setup go engine if requested
    Engine e;
    char e_col = 'w';
    e.pid = -1;
    if (flags.e_path)
    {
        StartEngine(&e, flags.e_path);
        strncpy(gameInfo->whiteName, e.name, 100);
        char** response = AllocateResponse();
        SendClearBoard(e.write, 1);
        if (!GetResponse(e.read, response, 1))
            fprintf(stderr, "Couldn't get response from engine\n");
        FreeResponse(response);
    }

    // Setup Networking if requested
    int client = -1;
    int host = -1;
    char host_col = 'w';
    char client_col = 'b';
    int opponents_turn = 0;
    if (flags.is_server && flags.port)
    {
        client = SetupServer(flags.port);
        opponents_turn = 1;
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
            snprintf(command, 10, "size %d", flags.size);
            SendCommand(client, command);
        }
        if (flags.handicap != 0)
        {
            char command[30] = "";
            snprintf(command, 30, "handicap %d", flags.handicap);
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
            if (resp == NULL)
            {
                printw("Connection to other user broken\n");
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
                ProcessCommand(&goban, host_col, resp);
            }
            free(resp);
        }
    }

    setlocale(LC_ALL, "");
    initscr();

    int running = 1;

    if (flags.sgf)
    {
        LoadSGF(&goban, flags.sgf);
        free(flags.sgf);
    }

    // Make sure board can fit screen
    if (!BoardFitsScreen(&goban)) 
    {
        while (!BoardFitsScreen(&goban))
            gameInfo->boardSize--;
        WriteNotes(
                 "Warning! Current size is too big for screen!\nMax size that "
                 "will fit is %d\n",
                 gameInfo->boardSize);
        gameInfo->boardSize = flags.size;
    }
    // Main loop
    while (running)
    {
        if (running == 2) // Game is in complete state
        {
            char resp[256];
            if (e.pid >= 0) // Ask engine to score board
            {
                char** response = AllocateResponse();
                SendFinalScore(e.write, 1);
                if (!GetResponse(e.read, response, 1))
                    fprintf(stderr, "Couldn't get response from engine\n");
                strcpy(gameInfo->result, response[1]);
                FreeResponse(response);
            }
            else // Manually score board
            {
                Goban tempgoban;
                memcpy(&tempgoban, &goban, sizeof(Goban));
                printw("Please enter a list of stones seperated by spaces\n"); 
                printw("(one stone per group)\n: ");
                getnstr(resp, 256);
                if (!RemoveDeadGroups(&tempgoban, resp))
                {
                    PrintDisplay(&goban);
                    continue;
                }
                else
                {
                    ScoreBoard(&tempgoban);
                    PrintDisplay(&tempgoban);
                    char resp[COMMAND_LENGTH];
                    mvprintw(getcury(stdscr), 0, "Does this look right?[Y/n]\n: ");
                    refresh();
                    getnstr(resp, 256);
                    if (resp[0] == 'n' || resp[0] == 'N')
                    {
                        PrintDisplay(&goban);
                        continue;
                    }
                    memcpy(&goban, &tempgoban, sizeof(Goban));
                }
                ScoreBoard(&goban);
                UpdateResult(&goban);
            }
            if (host >= 0 || client >= 0) // Make sure both players agree
            {
                printw("Confirming with opponent...\n");
                refresh();
                if (host >= 0) 
                {
                    SendCommand(host, resp);
                    char* confirm = RecvCommand(host);
                    if (!strcmp(confirm, "deny"))
                    {
                      WriteNotes("Opponent disagrees with result, play on.\n");
                      UndoHistory(&goban, 1);
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
                      WriteNotes("Opponent disagrees with result, play on.\n");
                        SendCommand(client, "deny");
                        UndoHistory(&goban, 1);
                        free(opponent_diff);
                        running = 1;
                        continue;
                    }
                    SendCommand(client, "confirm");
                    free(opponent_diff);
                }
            }
            AddHistory(&goban);
            PrintDisplay(&goban);
            running = 1;
        }

        PrintDisplay(&goban);
        PrintNotesw(&goban);
        refresh();
        if (e.pid >= 0 && (opponents_turn == 1))  // Engine's turn
        {
            char** response = AllocateResponse();

            if (GetHistorySize() == 1)
            {
                SendClearBoard(e.write, 1);
                if (!GetResponse(e.read, response, 1))
                    fprintf(stderr, "Couldn't get response from engine\n");
                SendBoardsize(e.write, 2, gameInfo->boardSize);
                if (!GetResponse(e.read, response, 2))
                    fprintf(stderr, "Couldn't get response from engine\n");
                SendKomi(e.write, 3, gameInfo->komi);
                if (!GetResponse(e.read, response, 3))
                    fprintf(stderr, "Couldn't get response from engine\n");
                SendHandicap(e.write, 4, gameInfo->komi);
                if (!GetResponse(e.read, response, 4))
                    fprintf(stderr, "Couldn't get response from engine\n");
            }

            SendPlay(e.write, 2, goban.lastmove, gameInfo->boardSize);
            if (!GetResponse(e.read, response, 2))
                fprintf(stderr, "Couldn't get response from engine\n");

            SendGenmove(e.write, 3, goban.color);
            if (!GetResponse(e.read, response, 3))
                fprintf(stderr, "Couldn't get response from engine\n");
            running = ProcessCommand(&goban, e_col, response[1]);
            if (running == MOVE)
            {
                SubmitMove(&goban, response[1]);
                opponents_turn = 0;
            }

            FreeResponse(response);
        } 
        else if ((host >= 0) || (client >= 0)) // If game is over network
        {
            if (GetHistorySize() == 1)
            {
                if (host >= 0)
                    opponents_turn = (goban.color == host_col) ? 1 : 0;
                if (client >= 0)
                    opponents_turn = (goban.color == client_col) ? 1 : 0;
            }
            struct pollfd inputs[2];
            inputs[0].fd = 0;
            inputs[0].events = POLLIN;
            inputs[1].fd = (host >= 0) ? host : client;
            inputs[1].events = POLLIN;
            if (opponents_turn)
                printw("Waiting on opponent...\n");
            mvprintw(getcury(stdscr), 0, ": ");
            echo();
            refresh();
            int ret_poll;
            while ((ret_poll = poll(inputs, 2, 100)) == 0); // Wait for input
            if (ret_poll > 0)
            {
                int user = (host >= 0) ? host : client;
                if (inputs[0].revents & POLLIN) // input from STDIN
                {
                    char input[COMMAND_LENGTH];
                    getnstr(input, COMMAND_LENGTH);
                    input[strcspn(input, "\n")] = '\0';

                    running = ProcessCommand(&goban, (host >= 0) ? client_col : host_col, input);
                    if (running == MOVE && !opponents_turn)
                    {
                        SubmitMove(&goban, input);
                        opponents_turn = 1;
                    }
                    if (IsNetworkedCommand(input))
                    { // This is a hack and I'm not proud
                        if (!strncmp(input, "undo", 4))
                        {
                            snprintf(input, COMMAND_LENGTH, "goto %d", GetViewIndex());
                            SendCommand(user, input);
                            snprintf(input, COMMAND_LENGTH, "undo here");
                            SendCommand(user, input);
                            if (host >= 0)
                                opponents_turn = (goban.color == host_col) ? 1 : 0;
                            if (client >= 0)
                                opponents_turn = (goban.color == client_col) ? 1 : 0;
                        }
                        else
                            SendCommand(user, input);
                    }
                }
                if (inputs[1].revents & POLLIN) // input from Network Socket
                {
                    char* response = RecvCommand(user);
                    if (response == NULL)
                    {
                        WriteNotes("Opponent disconnected\n");
                        printw("Opponent disconnected!\n");
                        printf("Opponent disconnected!\n");
                        host = -1;
                        client = -1;
                        free(response);
                        continue;
                    }
                    printw("%s\n", response);
                    running = ProcessCommand(&goban, (host >= 0) ? host_col : client_col, response);
                    if (running == MOVE && opponents_turn)
                    {
                        SubmitMove(&goban, response);
                        opponents_turn = 0;
                    }
                    if (!strncmp(response, "undo", 4))
                    {
                        if (host >= 0)
                            opponents_turn = (goban.color == host_col) ? 1 : 0;
                        if (client >= 0)
                            opponents_turn = (goban.color == client_col) ? 1 : 0;
                    }
                    free(response);
                }
            }
        }
        else // Playing alone
        {
            char input[COMMAND_LENGTH];
            mvprintw(getcury(stdscr), 0, ": ");
            refresh();
            getnstr(input, COMMAND_LENGTH);
            input[strcspn(input, "\n")] = 0;

            running = ProcessCommand(&goban, goban.color, input);
            if (running == MOVE)
            {
                SubmitMove(&goban, input);
                opponents_turn = 1;
            }
            input[0] = '\0';
        }

        if (running == SWAP) // Swap colors
        {
            char t = host_col;
            host_col = client_col;
            client_col = t;
            e_col = (e_col == 'w') ? 'b' : 'w';
            running = 1;
            char temp[100];
            strncpy(temp, gameInfo->whiteName, 100);
            strncpy(gameInfo->whiteName, gameInfo->blackName, 100);
            strncpy(gameInfo->blackName, temp, 100);
        }
    }
    endwin();
    FreeTree(GetRootNode());

    // Clean up
    if (host >= 0)
        CloseClient(host);
    if (client >= 0)
        CloseServer(client);
    return 0;
}
