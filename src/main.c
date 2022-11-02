#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "go.h"
#include "commands.h"

int main()
{
    srand(time(NULL));
    Goban goban;
    ResetGoban(&goban);
    goban.color = 'b';
    int running = 1;
    while (running)
    {
        PrintBoard(&goban);
        char input[COMMAND_LENGTH];
        if (!fgets(input, 256, stdin))
            exit(-1);
        running = ProcessCommand(&goban, input);
    }
    return 0;
}
