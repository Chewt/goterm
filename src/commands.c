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
    int is_networked;
    char* help;
};

int UndoCommand(Goban* goban, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1 || strcmp(tokens[0], "undo"))
        return -1;
    UndoHistory(goban);
    return 1;
}
int PrintCommand(Goban* goban, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1 || strcmp(tokens[0], "print"))
        return -1;
    PrintBoard(goban);
    return 1;
}
int ResetCommand(Goban* goban, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1 || strcmp(tokens[0], "reset"))
        return -1;
    ResetGoban(goban);
    return 1;
}
int SizeCommand(Goban* goban, int n_tokens, char tokens[][256])
{
    printf("%d tokens\n", n_tokens);
    if (n_tokens != 2 || strcmp(tokens[0], "size"))
        return -1;
    ResetGoban(goban);
    if((goban->size = atoi(tokens[1])) <= 1 || goban->size > 19)
        goban->size = 19;
    return 1;
}
int PassCommand(Goban* goban, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1 || strcmp(tokens[0], "pass"))
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
int SwapCommand(Goban* goban, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1 || strcmp(tokens[0], "swap"))
        return -1;
    return SWAP;
}
int ExitCommand(Goban* goban, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1 || strcmp(tokens[0], "exit"))
        return -1;
    goban->lastmove.p.col = -1;
    goban->lastmove.p.row = -1;
    return 0;
}
int ScoreCommand(Goban* goban, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1 || strcmp(tokens[0], "score"))
        return -1;
    ScoreBoard(goban);
    return 1;
}

int tokenize_command(char input[COMMAND_LENGTH], char tokens[][256])
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

    int terms = 0;
    token = strtok_r(input, " ", &save_ptr);
    if (token == NULL)
        return -1;
    while (token != NULL)
    {
        char* lowercase_token = to_lowercase(token);
        memcpy(tokens[terms], lowercase_token, strlen(lowercase_token) + 1);
        free(lowercase_token);
        token = strtok_r(NULL, " ", &save_ptr);
        terms++;
    }
    return terms;
}

struct GoCommand commands[] = {
    {"undo", UndoCommand, 1, "Undo last move"},
    {"print", PrintCommand, 0, "Print board to screen"},
    {"reset", ResetCommand, 1, "Reset board to empty"},
    {"size", SizeCommand, 1, "Change size of board. eg: size 19"},
    {"pass", PassCommand, 1, "Pass your turn"},
    {"swap", SwapCommand, 1, "Swap colors with opponent"},
    {"score", ScoreCommand, 0, "Show current score on board"},
    {"exit", ExitCommand, 1, "Exit program"},
    { 0 }
};

int is_networked_command(char input[COMMAND_LENGTH])
{
    int terms;
    char input_copy[COMMAND_LENGTH];
    memcpy(input_copy, input, COMMAND_LENGTH);
    char tokens[256][256];
    terms = tokenize_command(input_copy, tokens);
    if (terms <= 0)
        return 0;

    if (!strcmp(tokens[0], "help"))
        return 0;
    int i = 0;
    while (commands[i].name != NULL)
    {
        if (!strcmp(commands[i].name, tokens[0]))
        {
            return commands[i].is_networked;
        }
        i++;
    }
    return 1;
}

int ProcessCommand(Goban* goban, char input[COMMAND_LENGTH])
{
    int terms = 0;
    char tokens[256][256];
    terms = tokenize_command(input, tokens);

    int return_val = -2;
    int i;
    if (!strcmp(tokens[0], "help"))
    {
        i = 0;
        int chars_printed = 0;
        while (commands[i].name != NULL)
        {
            if (goban->notes == NULL)
            {
                printf("%s - %s\n", commands[i].name, commands[i].help);
            }
            else
            {
              chars_printed += snprintf(
                      goban->notes + chars_printed,
                      NOTES_LENGTH - chars_printed,
                      "%s - %s\n", commands[i].name, commands[i].help);
            }
            i++;
        }
        return 1;
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
                    if (goban->notes)
                    {
                      snprintf(goban->notes, NOTES_LENGTH,
                               "Invalid usage of command %s\n",
                               commands[i].name);
                    }
                    else
                        printf("Invalid usage of command %s\n", commands[i].name);
                    return 1;
                }
                return return_val;
            }
            ++i;
        }
    }
    return MOVE;
}

int SubmitMove(Goban* goban, char input[COMMAND_LENGTH])
{
    int terms = 0;
    char tokens[256][256];
    terms = tokenize_command(input, tokens);
    Point p;
    if (terms == 0 || !ValidateInput(goban, &p, tokens[0]))
    {
        if (goban->notes)
        {
            snprintf(goban->notes, NOTES_LENGTH - strlen(goban->notes),
                     "Invalid Input: %s\n", tokens[0]);
        }
        else
            printf("Invalid Input: %s\n", tokens[0]);
        return 1;
    }
    if (!ValidateMove(goban, p))
    {
        if (goban->notes)
            snprintf(goban->notes, NOTES_LENGTH,"Invalid Move\n");
        else
            printf("Invalid Move\n");
    }
    return 1;
}
