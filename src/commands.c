#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commands.h"
#include "go.h"

char* to_lowercase(char* s)
{
    if (!s)
        return NULL;
    char* ns = malloc(strlen(s) + 1);
    int pos = 0;
    char c;
    while((c = s[pos]) != '\0')
    {
        if (c == '\n')
            break;
        if (c >= 'A' && c <= 'Z')
            c += 32;
        ns[pos] = c;
        pos++;
    }
    ns[pos] = '\0';
    return ns;
}

int ProcessCommand(Goban* goban, char input[COMMAND_LENGTH])
{
    char* save_ptr;
    char* token;
    token = strtok_r(input, " ", &save_ptr);
    char* lowercase_token = to_lowercase(token);
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
        if (lowercase_token)
            free(lowercase_token);
        return 0;
    }
    else if (!strcmp(token, "size"))
    {
        token = strtok_r(NULL, " ", &save_ptr);
        if (!token)
        {
            printf("Invalid Input\n");
            if (lowercase_token)
                free(lowercase_token);
            return 1;
        }
        ResetGoban(goban);
        goban->size = atoi(token);
    }
    else if (!strcmp(lowercase_token, "pass"))
    {

        AddHistory(goban);
        if (goban->lastmove.p.row == -1 && goban->lastmove.p.col == -1)
        {
            if (lowercase_token)
                free(lowercase_token);
            return 2;
        }
        goban->lastmove.color = goban->color;
        goban->lastmove.p.row = -1;
        goban->lastmove.p.col = -1;
        goban->color = (goban->color == 'b') ? 'w' : 'b';
    }
    else
    {
        Point p;
        if (!ValidateInput(goban, &p, token))
        {
            printf("Invalid Input: %s\n", token);
            if (lowercase_token)
                free(lowercase_token);
            return 1;
        }
        if (!ValidateMove(goban, p))
            printf("Invalid Move\n");
    }
    if (lowercase_token)
        free(lowercase_token);
    return 1;
}
