#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <wchar.h>
#include "commands.h"
#include "go.h"
#include "sgf.h"

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

int RenameCommand(Goban* goban, int n_tokens, char tokens[][256])
{
    if (n_tokens < 3)
        return -1;
    char* color = to_lowercase(tokens[1]);

    char* name = NULL;
    if (!strcmp(color, "black"))
        name = goban->blackname;
    if (!strcmp(color, "white"))
        name = goban->whitename;
    if (name == NULL)
        return -1;
    int idx = 0;
    int i;
    for (i = 2; i < n_tokens; ++i)
    {
        idx += snprintf(name + idx, NAME_LENGTH, "%s ", tokens[i]);
    }
    name[idx - 1] = '\0';
    return 1;
}

int SGFCommand(Goban* goban, int n_tokens, char tokens[][256])
{
    if (n_tokens != 2 )
        return -1;
    char* sgf = CreateSGF();
    if (sgf == NULL)
    {
        WriteNotes(goban, "Please make at least one move before writing a sgf\n");
      return 1;
    }
    FILE* f = fopen(tokens[1], "w");
    fwrite(sgf, 1, strlen(sgf), f);
    fclose(f);
    free(sgf);
    WriteNotes(goban, "sgf saved to %s\n", tokens[1]);
    return 1;
}

int UndoCommand(Goban* goban, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1 && n_tokens != 2)
        return -1;
    if (n_tokens == 1)
        UndoHistory(goban, 1);
    else if (n_tokens == 2)
    {
        int n = 1;
        if (!strcmp(tokens[1], "here"))
            n = HistorySize() - (GetViewIndex() + 1);
        else
            n = strtol(tokens[1], NULL, 10);
        UndoHistory(goban, n);
    }
    return 1;
}

int GotoCommand(Goban* goban, int n_tokens, char tokens[][256])
{
    if (n_tokens != 2)
        return -1;
    else if (n_tokens == 2)
    {
        int n;
        if (!strcmp(tokens[1], "start"))
            n = 0;
        else if (!strcmp(tokens[1], "end"))
            n = HistorySize() - 1;
        else
            n = strtol(tokens[1], NULL, 10);
        n = (n >= 0) ? n : GetViewIndex();
        ViewHistory(goban, n);
    }
    return 1;
}

int NextCommand(Goban* goban, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1 && n_tokens != 2)
        return -1;
    if (n_tokens == 1)
        ViewHistory(goban, GetViewIndex() + 1);
    else if (n_tokens == 2)
    {
        int n = strtol(tokens[1], NULL, 10);
        ViewHistory(goban, GetViewIndex() + n);
    }
    return 1;
}

int BackCommand(Goban* goban, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1 && n_tokens != 2)
        return -1;
    if (n_tokens == 1)
        ViewHistory(goban, GetViewIndex() - 1);
    else if (n_tokens == 2)
    {
        int n = strtol(tokens[1], NULL, 10);
        ViewHistory(goban, GetViewIndex() - n);
    }
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
    if((goban->size = atoi(tokens[1])) <= 1 || goban->size > 19)
        goban->size = 19;
    if (!BoardFitsScreen(goban))
    {
        int t = goban->size;
        while (!BoardFitsScreen(goban))
            goban->size--;
        WriteNotes(goban,
                 "Warning! Current size is too big for screen!\nMax size that "
                 "will fit is %d\n",
                 goban->size);
        goban->size = t;
    }
    return 1;
}
int PassCommand(Goban* goban, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1)
        return -1;
    if (goban->lastmove.p.row == -1 && goban->lastmove.p.col == -1)
        return 2;
    goban->lastmove.color = goban->color;
    goban->lastmove.p.row = -1;
    goban->lastmove.p.col = -1;
    goban->color = (goban->color == 'b') ? 'w' : 'b';
    AddHistory(goban);
    return 1;
}
int SwapCommand(Goban* goban, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1)
        return -1;
    return SWAP;
}
int ExitCommand(Goban* goban, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1)
        return -1;
    goban->lastmove.p.col = -1;
    goban->lastmove.p.row = -1;
    return 0;
}
int ScoreCommand(Goban* goban, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1)
        return -1;
    ScoreBoard(goban);
    return 1;
}

int KomiCommand(Goban* goban, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1 && n_tokens != 2)
        return -1;
    if (n_tokens == 1)
        WriteNotes(goban, "Komi is %.1f\n", goban->komi);
    else if (n_tokens == 2)
    {
        ResetGoban(goban);
        goban->komi = atof(tokens[1]);
    }
    return 1;
}

int HandicapCommand(Goban* goban, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1 && n_tokens != 2)
        return -1;
    if (n_tokens == 1)
        WriteNotes(goban, "Handicap is %d\n", goban->handicap);
    else if (n_tokens == 2)
    {
        ResetGoban(goban);
        goban->handicap = atoi(tokens[1]);
        SetHandicap(goban, goban->handicap);
        goban->color = 'w';
    }
    return 1;
}
int SayCommand(Goban* goban, int n_tokens, char tokens[][256])
{
    if (n_tokens <= 1)
        return -1;
    int i;
    WriteNotes(goban, "Message: \"");
    for (i = 1; i < n_tokens; ++i)
    {
        AppendNotes(goban, "%s ", tokens[i]);
    }
    AppendNotes(goban, "\"\n");
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
        if (terms == 0) // only want first token to be lowercase
        {
            char* lowercase_token = to_lowercase(token);
            memcpy(tokens[terms], lowercase_token, strlen(lowercase_token) + 1);
            free(lowercase_token);
        }
        else
            memcpy(tokens[terms], token, strlen(token) + 1);
        token = strtok_r(NULL, " ", &save_ptr);
        terms++;
    }
    return terms;
}

struct GoCommand commands[] = {
    {"undo", UndoCommand, 1, "Undo last move.\nOptionally you can supply a number of moves to undo, or the word \"here\" to undo to the board state currently in view"},
    {"reset", ResetCommand, 1, "Reset board to empty"},
    {"size", SizeCommand, 1, "Change size of board. eg: size 19"},
    {"pass", PassCommand, 1, "Pass your turn"},
    {"swap", SwapCommand, 1, "Swap colors with opponent"},
    {"score", ScoreCommand, 0, "Show current score on board"},
    {"komi", KomiCommand, 1, "Show current komi or set new komi"},
    {"say", SayCommand, 1, "Send a message to other player"},
    {"handicap", HandicapCommand, 1, "Set handicap on board."},
    {"rename", RenameCommand, 1, "Change the name of black or white player\nUsage: rename black|white NAME"},
    {"sgf", SGFCommand, 0, "Saves the current sgf to a file\nUsage: sgf FILENAME"},
    {"next", NextCommand, 0, "Shows the next move"},
    {"back", BackCommand, 0, "Shows the previous move"},
    {"goto", GotoCommand, 0, "Go to a specific move in the game"},
    {"exit", ExitCommand, 0, "Exit program"},
    { 0 }
};

int AutoComplete(char* input)
{
    int matches = 0;
    int last_match = -1;
    int i = 0;
    while (commands[i].name != NULL)
    {
        if (strlen(commands[i].name) < strlen(input))
        {
            i++;
            continue;
        }
        if (!strncmp(input, commands[i].name, strlen(input)))
        {
            last_match = i;
            matches++;
        }
        i++;
    }
    if (matches == 1)
        return last_match;
    if (matches > 1)
        return -2;
    return -1;
}

int IsNetworkedCommand(char input[COMMAND_LENGTH])
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
    int idx = AutoComplete(tokens[0]);
    if (idx >= 0)
        return commands[idx].is_networked;
    return 1;
}

int ProcessCommand(Goban* goban, char input[COMMAND_LENGTH])
{
    char input_copy[COMMAND_LENGTH];
    memcpy(input_copy, input, COMMAND_LENGTH);
    int terms = 0;
    char tokens[256][256];
    terms = tokenize_command(input_copy, tokens);
    if (terms < 0)
        return 1;

    int return_val = -2;
    int i;
    if (!strcmp(tokens[0], "help"))
    {
        i = 0;
        while (commands[i].name != NULL)
        {
            if (terms > 1 && !strcmp(tokens[1], commands[i].name))
            {
                WriteNotes(goban, "%s - %s\n", commands[i].name, commands[i].help);
                return 1;
            }
            if (goban->notes == NULL)
                printf("%s - %s\n", commands[i].name, commands[i].help);
            else
                AppendNotes(goban, "%s%c", commands[i].name, (i % 3 == 2) ? '\n' : '\t');
            i++;
        }
        AppendNotes(goban, "\n");
        return 1;
    }
    else // Try commands
    {
        i = AutoComplete(tokens[0]);
        if (i >= 0)
        {
            return_val = commands[i].func(goban, terms, tokens);
            if (return_val == -1)
            {
                WriteNotes(goban, "Invalid usage of command %s\n", commands[i].name);
                return 1;
            }
            return return_val;
        }
        else if (i == -2)
        {
            WriteNotes(goban, "Command is ambiguous\n");
            return 1;
        }
    }
    return MOVE;
}

int SubmitMove(Goban* goban, char input[COMMAND_LENGTH])
{
    if (GetViewIndex() != (HistorySize() - 1))
        ViewHistory(goban, HistorySize() - 1);
    int terms = 0;
    char tokens[256][256];
    terms = tokenize_command(input, tokens);
    Move m;
    m.color = goban->color;
    if (terms == 0 || !ValidateInput(goban, &m.p, tokens[0]))
    {
        if (goban->notes)
        {
            WriteNotes(goban, "Invalid Input: %s\n", tokens[0]);
        }
        else
            printf("Invalid Input: %s\n", tokens[0]);
        return 1;
    }
    if (!ValidateMove(goban, m))
    {
        if (goban->notes)
            WriteNotes(goban, "Invalid Move\n");
        else
            printf("Invalid Move\n");
    }
    return 1;
}
