#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commands.h"
#include "go.h"

int ProcessCommand(Goban* goban, char input[COMMAND_LENGTH])
{
    if (!strcmp(input, "undo\n"))
    {
        UndoHistory(goban);
    }
    else if (!strcmp(input, "print\n"))
    {
        PrintBoard(goban);
    }
    else if (!strcmp(input, "reset\n"))
    {
        ResetGoban(goban);
    }
    else if (!strcmp(input, "exit\n"))
    {
        return 0;
    }
    else
    {
        Point p;
        if (!ValidateInput(&p, input))
        {
            printf("Invalid Input\n");
            return 1;
        }
        if (!ValidateMove(goban, p))
            printf("Invalid Move\n");
    }
    return 1;
}
