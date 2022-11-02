#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commands.h"
#include "go.h"

int ProcessCommand(Goban* goban, char input[COMMAND_LENGTH])
{
    char* save_ptr;
    char* token;
    token = strtok_r(input, " ", &save_ptr);
    if (!strcmp(token, "undo\n"))
    {
        UndoHistory(goban);
    }
    else if (!strcmp(token, "print\n"))
    {
        PrintBoard(goban);
    }
    else if (!strcmp(token, "reset\n"))
    {
        ResetGoban(goban);
    }
    else if (!strcmp(token, "exit\n"))
    {
        return 0;
    }
    else if (!strcmp(token, "size"))
    {
        token = strtok_r(NULL, " ", &save_ptr);
        if (!token)
        {
            printf("Invalid Input\n");
            return 1;
        }
        ResetGoban(goban);
        goban->size = atoi(token);
    }
    else
    {
        Point p;
        if (!ValidateInput(goban, &p, token))
        {
            printf("Invalid Input\n");
            return 1;
        }
        if (!ValidateMove(goban, p))
            printf("Invalid Move\n");
    }
    return 1;
}
