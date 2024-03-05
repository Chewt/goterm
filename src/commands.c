#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <wchar.h>
#include "commands.h"
#include "gametree.h"
#include "go.h"
#include "sgf.h"
#include "gameinfo.h"
#include "display.h"


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

int CursorCommand(Goban* goban, char player, int n_tokens, char tokens[][256])
{
    if (n_tokens < 2)
        return -1;
    if (n_tokens == 2)
    {
        if (!strcmp("left", tokens[1]))
            MoveCursor(-1, 0);
        else if (!strcmp("right", tokens[1]))
            MoveCursor(1, 0);
        else if (!strcmp("up", tokens[1]))
            MoveCursor(0, -1);
        else if (!strcmp("down", tokens[1]))
            MoveCursor(0, 1);
        else if (!strcmp("place", tokens[1]))
        {
            Point* cursor = GetCursor();
            Move m;
            m.p.col = cursor->col;
            m.p.row = cursor->row;
            m.color = goban->color;
            ValidateMove(goban, m);
        }
        else
            return -1;
        return 1;
    }
    if (n_tokens < 3)
        return -1;
    if (!strcmp("star", tokens[1]))
    {
        GameInfo* gameInfo = GetGameInfo();
        Point* cursor = GetCursor();
        if (!strcmp("left", tokens[2]))
        {
            if (cursor->col <= ((gameInfo->boardSize - 1) / 2))
                SetCursor(3, cursor->row);
            else if (cursor->col > (gameInfo->boardSize - 4))
                SetCursor(gameInfo->boardSize - 4, cursor->row);
            else
                SetCursor((gameInfo->boardSize - 1) / 2, cursor->row);
        }
        else if (!strcmp("right", tokens[2]))
        {
            if (cursor->col >= ((gameInfo->boardSize - 1) / 2))
                SetCursor(gameInfo->boardSize - 4, cursor->row);
            else if (cursor->col >= 3)
                SetCursor((gameInfo->boardSize - 1) / 2, cursor->row);
            else
                SetCursor( 3, cursor->row);
        }
        else if (!strcmp("up", tokens[2]))
        {
            if (cursor->row <= ((gameInfo->boardSize - 1) / 2))
                SetCursor(cursor->col, 3);
            else if (cursor->row > (gameInfo->boardSize - 4))
                SetCursor(cursor->col, gameInfo->boardSize - 4);
            else
                SetCursor(cursor->col, (gameInfo->boardSize - 1) / 2);
        }
        else if (!strcmp("down", tokens[2]))
        {
            if (cursor->row >= ((gameInfo->boardSize - 1) / 2))
                SetCursor(cursor->col, gameInfo->boardSize - 4);
            else if (cursor->row >= 3)
                SetCursor(cursor->col, (gameInfo->boardSize - 1) / 2);
            else
                SetCursor(cursor->col, 3);
        }
        else
            return -1;
        return 1;
    }
    if (n_tokens < 4)
        return -1;
    if (!strcmp("set", tokens[1]))
    {
        int x, y;
        x = strtol(tokens[2], NULL, 10);
        y = strtol(tokens[3], NULL, 10);
        SetCursor(x, y);
    }
    else
        return -1;
    return 1;
}

int JumpCommand(Goban* goban, char player, int n_tokens, char tokens[][256])
{
    if (n_tokens < 2)
        return -1;
    char* direction = to_lowercase(tokens[1]);
    if (!strcmp(direction, "up"))
        JumpBranch(goban, -1);
    else if (!strcmp(direction, "down"))
        JumpBranch(goban, 1);
    else if (!strcmp(direction, "next"))
    {
        int i = 1;
        GameNode* node = GetViewedNode()->mainline_next;
        while (node && !node->n_alts)
        {
            node = node->mainline_next;
            i++;
        }
        if (!node)
            i--;
        SlideHistory(goban, i);
    }
    else if (!strcmp(direction, "back"))
    {
        int i = 1;
        GameNode* node = GetViewedNode()->mainline_prev;
        while (node && !node->n_alts)
        {
            node = node->mainline_prev;
            i++;
        }
        if (!node)
            i--;
        SlideHistory(goban, -i);
    }
    else 
    {
        free(direction);
        return -1;
    }
    free(direction);
    return 1;
}

int RenameCommand(Goban* goban, char player, int n_tokens, char tokens[][256])
{
    if (n_tokens < 3)
        return -1;
    char* color = to_lowercase(tokens[1]);
    GameInfo* gi = GetGameInfo();
    char* name = NULL;
    if (!strcmp(color, "black"))
        name = gi->blackName;
    if (!strcmp(color, "white"))
        name = gi->whiteName;
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

int SGFCommand(Goban* goban, char player, int n_tokens, char tokens[][256])
{
    if (n_tokens != 2 )
        return -1;
    char* sgf = CreateSGF();
    if (sgf == NULL)
    {
        WriteNotes("Please make at least one move before writing a sgf\n");
      return 1;
    }
    FILE* f = fopen(tokens[1], "w");
    fwrite(sgf, 1, strlen(sgf), f);
    fclose(f);
    free(sgf);
    WriteNotes("sgf saved to %s\n", tokens[1]);
    return 1;
}

int UndoCommand(Goban* goban, char player, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1 && n_tokens != 2)
        return -1;
    if (n_tokens == 1)
        UndoHistory(goban, 1);
    else if (n_tokens == 2)
    {
        int n = 1;
        if (!strcmp(tokens[1], "here"))
            n = GetHistorySize() - (GetViewIndex() + 1);
        else
            n = strtol(tokens[1], NULL, 10);
        UndoHistory(goban, n);
    }
    return 1;
}

int GotoCommand(Goban* goban, char player, int n_tokens, char tokens[][256])
{
    if (n_tokens != 2)
        return -1;
    else if (n_tokens == 2)
    {
        int n;
        if (!strcmp(tokens[1], "start"))
            n = 0;
        else if (!strcmp(tokens[1], "end"))
            n = GetHistorySize() - 1;
        else
            n = strtol(tokens[1], NULL, 10);
        n = (n >= 0) ? n : GetViewIndex();
        ViewHistory(goban, n);
    }
    return 1;
}

int NextCommand(Goban* goban, char player, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1 && n_tokens != 2)
        return -1;
    if (n_tokens == 1)
        SlideHistory(goban, 1);
    else if (n_tokens == 2)
    {
        int n = strtol(tokens[1], NULL, 10);
        SlideHistory(goban, n);
    }
    return 1;
}

int BackCommand(Goban* goban, char player, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1 && n_tokens != 2)
        return -1;
    if (n_tokens == 1)
        SlideHistory(goban, -1);
    else if (n_tokens == 2)
    {
        int n = strtol(tokens[1], NULL, 10);
        SlideHistory(goban, -n);
    }
    return 1;
}
int ResetCommand(Goban* goban, char player, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1)
        return -1;
    ResetGoban(goban);
    return 1;
}
int SizeCommand(Goban* goban, char player, int n_tokens, char tokens[][256])
{
    if (n_tokens != 2)
        return -1;
    ResetGoban(goban);
    GameInfo* gameInfo = GetGameInfo();
    if((gameInfo->boardSize = atoi(tokens[1])) <= 1 || gameInfo->boardSize > 19)
        gameInfo->boardSize = 19;
    if (!BoardFitsScreen(goban))
    {
        int t = gameInfo->boardSize;
        while (!BoardFitsScreen(goban))
            gameInfo->boardSize--;
        WriteNotes(
                 "Warning! Current size is too big for screen!\nMax size that "
                 "will fit is %d\n",
                 gameInfo->boardSize);
        gameInfo->boardSize = t;
    }
    return 1;
}
int PassCommand(Goban* goban, char player, int n_tokens, char tokens[][256])
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
int ResignCommand(Goban* goban, char player, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1)
        return -1;
    GameInfo* gameInfo = GetGameInfo();
    snprintf(gameInfo->result, RESULT_LENGTH, "%c+Resign", (player == 'b') ? 'W' : 'B');
    WriteNotes("%s Resigned\n", (player == 'b') ? gameInfo->blackName : gameInfo->whiteName);
    return 1;
}
int SwapCommand(Goban* goban, char player, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1)
        return -1;
    return SWAP;
}
int ExitCommand(Goban* goban, char player, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1)
        return -1;
    goban->lastmove.p.col = -1;
    goban->lastmove.p.row = -1;
    return 0;
}
int ScoreCommand(Goban* goban, char player, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1)
        return -1;
    ScoreBoard(goban);
    return 1;
}

int KomiCommand(Goban* goban, char player, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1 && n_tokens != 2)
        return -1;
    GameInfo* gameInfo = GetGameInfo();
    if (n_tokens == 1)
        WriteNotes("Komi is %.1f\n", gameInfo->komi);
    else if (n_tokens == 2)
    {
        ResetGoban(goban);
        gameInfo->komi = atof(tokens[1]);
    }
    return 1;
}

int HandicapCommand(Goban* goban, char player, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1 && n_tokens != 2)
        return -1;
    GameInfo* gameInfo = GetGameInfo();
    if (n_tokens == 1)
        WriteNotes("Handicap is %d\n", gameInfo->handicap);
    else if (n_tokens == 2)
    {
        ResetGoban(goban);
        gameInfo->handicap = atoi(tokens[1]);
        SetHandicap(goban, gameInfo->handicap);
        goban->color = 'w';
    }
    return 1;
}
int SayCommand(Goban* goban, char player, int n_tokens, char tokens[][256])
{
    if (n_tokens <= 1)
        return -1;
    GameInfo* gameInfo = GetGameInfo();
    GameNode* current_node = GetViewedNode();
    int i;
    AppendComment(current_node, "%s: \"", (player == 'b') ? gameInfo->blackName : gameInfo->whiteName);
    for (i = 1; i < n_tokens - 1; ++i)
    {
        AppendComment(current_node, "%s ", tokens[i]);
    }
    AppendComment(current_node, "%s", tokens[i]);
    AppendComment(current_node, "\"\n");
    return 1;
}

/* DEBUG COMMANDS */
int ToggleBoardCommand(Goban* goban, char player, int n_tokens, char tokens[][256])
{
    DisplayConfig* dc = GetDisplayConfig();
    dc->showBoard ^= 1;
    return 1;
}
int ToggleInfoCommand(Goban* goban, char player, int n_tokens, char tokens[][256])
{
    DisplayConfig* dc = GetDisplayConfig();
    dc->showInfo ^= 1;
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

struct GoCommand
{
    char* name;
    int (*func)(Goban* goban, char player, int n_tokens, char tokens[][256]);
    int is_networked;
    int is_repeatable;
    char* help;
};

// name, function, is_networked, is_repeatable, help
struct GoCommand commands[] = {
    {"undo", UndoCommand, 1, 0, "Undo last move.\nOptionally you can supply a number of moves to undo, or the word \"here\" to undo to the board state currently in view"},
    {"reset", ResetCommand, 1, 0, "Reset board to empty"},
    {"size", SizeCommand, 1, 0, "Change size of board. eg: size 19"},
    {"pass", PassCommand, 1, 0, "Pass your turn"},
    {"swap", SwapCommand, 1, 0, "Swap colors with opponent"},
    {"score", ScoreCommand, 0, 0, "Show current score on board"},
    {"komi", KomiCommand, 1, 0, "Show current komi or set new komi"},
    {"say", SayCommand, 1, 0, "Send a message to other player"},
    {"handicap", HandicapCommand, 1, 0, "Set handicap on board."},
    {"rename", RenameCommand, 1, 0, "Change the name of black or white player\nUsage: rename black|white NAME"},
    {"resign", ResignCommand, 1, 0, "Resign"},
    {"sgf", SGFCommand, 0, 0, "Saves the current sgf to a file\nUsage: sgf FILENAME"},
    {"next", NextCommand, 0, 1, "Shows the next move"},
    {"back", BackCommand, 0, 1, "Shows the previous move"},
    {"jump", JumpCommand, 0, 1, "Jump between branches. Options are [up|down|next|back]"},
    {"goto", GotoCommand, 0, 0, "Go to a specific move in the game"},
    {"cursor", CursorCommand, 0, 0, "Move the cursor. Options are [up|down|left|right|set] where set takes a column and a row"},
    {"exit", ExitCommand, 0, 0, "Exit program"},
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

int ProcessCommand(Goban* goban, char player, char input[COMMAND_LENGTH])
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
                WriteNotes("%s - %s\n", commands[i].name, commands[i].help);
                return 1;
            }
            AppendNotes("%s%c", commands[i].name, (i % 3 == 2) ? '\n' : '\t');
            i++;
        }
        AppendNotes("\n");
        return 1;
    }
    else // Try commands
    {
        i = AutoComplete(tokens[0]);
        if (i >= 0)
        {
            return_val = commands[i].func(goban, player, terms, tokens);
            if (return_val == -1)
            {
                WriteNotes("Invalid usage of command %s\n", commands[i].name);
                return 1;
            }
            return return_val;
        }
        else if (i == -2)
        {
            WriteNotes("Command is ambiguous\n");
            return 1;
        }
    }
    return MOVE;
}

int SubmitMove(Goban* goban, char input[COMMAND_LENGTH])
{
    int terms = 0;
    char tokens[256][256];
    terms = tokenize_command(input, tokens);
    Move m;
    m.color = goban->color;
    if (terms == 0 || !ValidateInput(goban, &m.p, tokens[0]))
    {
        WriteNotes("Invalid Input: %s\n", tokens[0]);
        return 1;
    }
    GameInfo* gameInfo = GetGameInfo();
    if (!gameInfo->can_edit && GetViewIndex() != (GetHistorySize() - 1))
    {
        ViewHistory(goban, GetHistorySize() - 1);
        m.color = (m.color == 'b') ? 'w' : 'b';
    }
    if (!ValidateMove(goban, m))
    {
        AppendNotes("Invalid Move\n");
    }
    return 1;
}
