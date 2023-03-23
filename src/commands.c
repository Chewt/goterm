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

struct GoCommand
{
    char* name;
    int (*func)(Goban* goban, int n_tokens, char tokens[][256]);
    char* help;
};

int UndoCommand(Goban* goban, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1)
        return -1;
    UndoHistory(goban);
    return 1;
}
int PrintCommand(Goban* goban, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1)
        return -1;
    PrintBoard(goban);
    return 1;
}
int ResetCommand(Goban* goban, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1)
        return -1;
    ResetGoban(goban);
    return 1;
}
int SizeCommand(Goban* goban, int n_tokens, char tokens[][256])
{
    if (n_tokens != 2)
        return -1;
    ResetGoban(goban);
    goban->size = atoi(tokens[1]);
    return 1;
}
int PassCommand(Goban* goban, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1)
        return -1;
    AddHistory(goban);
    if (goban->lastmove.p.row == -1 && goban->lastmove.p.col == -1)
        return 2;
    goban->lastmove.color = goban->color;
    goban->lastmove.p.row = -1;
    goban->lastmove.p.col = -1;
    goban->color = (goban->color == 'b') ? 'w' : 'b';
    return 1;
}
int ExitCommand(Goban* goban, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1)
        return -1;
    return 0;
}

int ProcessCommand(Goban* goban, char input[COMMAND_LENGTH])
{
    char* save_ptr;
    char* token;

    int i = 0;
    while (input[i] != '\0')
    {
        if (input[i] == '\n')
            input[i] = '\0';
        else
            i++;
    }

    struct GoCommand commands[] = {
    {"undo", UndoCommand, "Undo last move"},
    {"print", PrintCommand, "Print board to screen"},
    {"reset", ResetCommand, "Reset board to empty"},
    {"size", SizeCommand, "Change size of board. eg: size 19"},
    {"pass", PassCommand, "Pass your turn"},
    {"exit", ExitCommand, "Exit program"},
    { 0 }
    };

    int terms = 0;
    char tokens[256][256];
    token = strtok_r(input, " ", &save_ptr);
    if (token == NULL)
        return 1;
    while (token != NULL)
    {
        char* lowercase_token = to_lowercase(token);
        memcpy(tokens[terms], lowercase_token, strlen(lowercase_token) + 1);
        free(lowercase_token);
        token = strtok_r(NULL, " ", &save_ptr);
        terms++;
    }

    int return_val = -2;
    if (!strcmp(tokens[0], "help"))
    {
        i = 0;
        while (commands[i].name != NULL)
        {
            printf("%s \t %s\n", commands[i].name, commands[i].help);
            i++;
        }
    }
    else
    {
        i = 0;
        while (commands[i].name != NULL)
        {
            if (!strcmp(commands[i].name, tokens[0]))
            {
                return_val = commands[i].func(goban, terms, tokens);
                if (return_val == -1)
                {
                    printf("Invalid usage of command %s\n", commands[i].name);
                    return 1;
                }
                return return_val;
            }
            ++i;
        }
    }
    Point p;
    if (!ValidateInput(goban, &p, tokens[0]))
    {
        printf("Invalid Input: %s\n", tokens[0]);
        return 1;
    }
    if (!ValidateMove(goban, p))
        printf("Invalid Move\n");
    return 1;
}
