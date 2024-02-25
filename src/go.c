#include <stdio.h>
#include <string.h>
#include "go.h"
#include "gameinfo.h"
#include "gametree.h"
#include "stack.h"
#include "commands.h"

// Return 0 if input isn't in move format, else returns 1 and populates p
int ValidateInput(Goban* goban, Point* p, char input[256])
{
    GameInfo* gameInfo = GetGameInfo();
    char col = input[0];
    int row = input[1] - '0';
    int i;
    for (i = 2; i < 256 && row > 0 && row < gameInfo->boardSize; ++i)
    {
        if (input[i] == '\n' || input[i] == '\0')
            break;
        row *= 10;
        row += input[i] - '0';
    }
    if (col >= 'A' && col <= 'T')
        col += 32;
    if (col >= 'a' && col <= 'h')
        col -= 'a';
    else if (col > 'i' && col <= 't')
        col -= 'a' + 1;
    else
    {
        return 0;
    }
    if (row >= 1 && row <= gameInfo->boardSize)
        row = gameInfo->boardSize - row;
    else
        return 0;
    if (col < 0 || col > gameInfo->boardSize)
        return 0;
    p->row = row;
    p->col = col;
    return 1;
}


void ResetGoban(Goban* goban)
{
    ClearBoard(goban);
    goban->wpris = 0;
    goban->bpris = 0;
    goban->color = 'b';
    goban->lastmove.color = 'b';
    goban->lastmove.p.col = -2;
    goban->lastmove.p.row = -2;
    goban->showscore = 0;
    NewTree(goban);
}

void ClearBoard(Goban* goban)
{
    int i, j;
    for (i = 0; i < 19; ++i)
        for (j = 0; j < 19; ++j)
        {
            goban->board[i][j] = ' ';
            goban->score[i][j] = ' ';
        }
}

void SetHandicap(Goban* goban, int numStones)
{
    GameInfo* gameInfo = GetGameInfo();
    int line = (gameInfo->boardSize <= 9) ? 3 : 4;
    switch (numStones)
    {
        case 1:
            goban->board[gameInfo->boardSize / 2][gameInfo->boardSize / 2] = 'b';
            break;
        case 2:
            goban->board[gameInfo->boardSize - line][line - 1] = 'b';
            goban->board[line - 1][gameInfo->boardSize - line] = 'b';
            break;
        case 3:
            goban->board[line - 1][line - 1] = 'b';
            goban->board[gameInfo->boardSize - line][line - 1] = 'b';
            goban->board[line - 1][gameInfo->boardSize - line] = 'b';
            break;
        case 4:
            goban->board[line - 1][line - 1] = 'b';
            goban->board[gameInfo->boardSize - line][line - 1] = 'b';
            goban->board[line - 1][gameInfo->boardSize - line] = 'b';
            goban->board[gameInfo->boardSize - line][gameInfo->boardSize - line] = 'b';
            break;
        case 5:
            goban->board[line - 1][line - 1] = 'b';
            goban->board[gameInfo->boardSize - line][line - 1] = 'b';
            goban->board[line - 1][gameInfo->boardSize - line] = 'b';
            goban->board[gameInfo->boardSize - line][gameInfo->boardSize - line] = 'b';
            goban->board[gameInfo->boardSize / 2][gameInfo->boardSize / 2] = 'b';
            break;
        case 6:
            goban->board[line - 1][line - 1] = 'b';
            goban->board[gameInfo->boardSize - line][line - 1] = 'b';
            goban->board[line - 1][gameInfo->boardSize - line] = 'b';
            goban->board[gameInfo->boardSize - line][gameInfo->boardSize - line] = 'b';
            goban->board[line - 1][gameInfo->boardSize / 2] = 'b';
            goban->board[gameInfo->boardSize - line][gameInfo->boardSize / 2] = 'b';
            break;
        case 7:
            goban->board[line - 1][line - 1] = 'b';
            goban->board[gameInfo->boardSize - line][line - 1] = 'b';
            goban->board[line - 1][gameInfo->boardSize - line] = 'b';
            goban->board[gameInfo->boardSize - line][gameInfo->boardSize - line] = 'b';
            goban->board[line - 1][gameInfo->boardSize / 2] = 'b';
            goban->board[gameInfo->boardSize - line][gameInfo->boardSize / 2] = 'b';
            goban->board[gameInfo->boardSize / 2][gameInfo->boardSize / 2] = 'b';
            break;
        case 8:
            goban->board[line - 1][line - 1] = 'b';
            goban->board[gameInfo->boardSize - line][line - 1] = 'b';
            goban->board[line - 1][gameInfo->boardSize - line] = 'b';
            goban->board[gameInfo->boardSize - line][gameInfo->boardSize - line] = 'b';
            goban->board[line - 1][gameInfo->boardSize / 2] = 'b';
            goban->board[gameInfo->boardSize - line][gameInfo->boardSize / 2] = 'b';
            goban->board[gameInfo->boardSize / 2][line - 1] = 'b';
            goban->board[gameInfo->boardSize / 2][gameInfo->boardSize - line] = 'b';
            break;
        case 9:
            goban->board[line - 1][line - 1] = 'b';
            goban->board[gameInfo->boardSize - line][line - 1] = 'b';
            goban->board[line - 1][gameInfo->boardSize - line] = 'b';
            goban->board[gameInfo->boardSize - line][gameInfo->boardSize - line] = 'b';
            goban->board[line - 1][gameInfo->boardSize / 2] = 'b';
            goban->board[gameInfo->boardSize - line][gameInfo->boardSize / 2] = 'b';
            goban->board[gameInfo->boardSize / 2][line - 1] = 'b';
            goban->board[gameInfo->boardSize / 2][gameInfo->boardSize - line] = 'b';
            goban->board[gameInfo->boardSize / 2][gameInfo->boardSize / 2] = 'b';
            break;
    }
}

int IsSeen(Point seen[361], int nseen, Point p)
{
    int i;
    for (i = 0; i < nseen; ++i)
    {
        Point curr = seen[i];
        if ((curr.row == p.row) && (curr.col == p.col))
            return 1;
    }
    return 0;
}

int CountLiberties(Goban* goban, Point start, char searched[19][19])
{
    Stack stack;
    Point seen[361];
    int seenSize = 0;
    int liberties = 0;
    char color = goban->board[start.row][start.col];

    GameInfo* gameInfo = GetGameInfo();
    memset(seen, -1, sizeof(Point) * 361);
    ClearStack(&stack);
    PushStack(&stack, start);
    while (StackSize(&stack) > 0)
    {
        Point currentPoint = PopStack(&stack);
        Point tempPoint = currentPoint;
        seen[seenSize++] = currentPoint;

        if (currentPoint.row > 0)
        {
            tempPoint.row = currentPoint.row - 1;
            tempPoint.col = currentPoint.col;
            if (goban->board[tempPoint.row][tempPoint.col] == color &&
                    !IsSeen(seen, seenSize, tempPoint))
            {
                PushStack(&stack, tempPoint);
            }
            else if (goban->board[currentPoint.row - 1][currentPoint.col] == ' ')
                liberties++;
        }
        if (currentPoint.row < gameInfo->boardSize - 1)
        {
            tempPoint.row = currentPoint.row + 1;
            tempPoint.col = currentPoint.col;
            if (goban->board[tempPoint.row][tempPoint.col] == color &&
                    !IsSeen(seen, seenSize, tempPoint))
            {
                PushStack(&stack, tempPoint);
            }
            else if (goban->board[currentPoint.row + 1][currentPoint.col] == ' ')
                liberties++;
        }
        if (currentPoint.col > 0)
        {
            tempPoint.row = currentPoint.row;
            tempPoint.col = currentPoint.col - 1;
            if (goban->board[tempPoint.row][tempPoint.col] == color &&
                    !IsSeen(seen, seenSize, tempPoint))
            {
                PushStack(&stack, tempPoint);
            }
            else if (goban->board[currentPoint.row][currentPoint.col - 1] == ' ')
                liberties++;
        }
        if (currentPoint.col < gameInfo->boardSize - 1)
        {
            tempPoint.row = currentPoint.row;
            tempPoint.col = currentPoint.col + 1;
            if (goban->board[tempPoint.row][tempPoint.col] == color &&
                    !IsSeen(seen, seenSize, tempPoint))
            {
                PushStack(&stack, tempPoint);
            }
            else if (goban->board[currentPoint.row][currentPoint.col + 1] == ' ')
                liberties++;
        }
    }
    int i;
    if (liberties > 0)
    {
        for (i = 0; i < seenSize; ++i)
        {
            searched[seen[i].row][seen[i].col] = 'x';
        }
    }
    return liberties;
}

int RemoveGroup(Goban* goban, Point start)
{
    GameInfo* gameInfo = GetGameInfo();
    Stack stack;
    Point seen[361];
    int seenSize = 0;
    char color = goban->board[start.row][start.col];

    memset(seen, ' ', sizeof(Point) * 361);
    ClearStack(&stack);
    PushStack(&stack, start);
    seen[seenSize++] = start;
    while (StackSize(&stack) > 0)
    {
        Point currentPoint = PopStack(&stack);
        Point tempPoint = currentPoint;

        if (currentPoint.row > 0)
        {
            tempPoint.row = currentPoint.row - 1;
            tempPoint.col = currentPoint.col;
            if (goban->board[tempPoint.row][tempPoint.col] == color &&
                    !IsSeen(seen, seenSize, tempPoint))
            {
                PushStack(&stack, tempPoint);
                seen[seenSize++] = tempPoint;
            }
        }
        if (currentPoint.row < gameInfo->boardSize - 1)
        {
            tempPoint.row = currentPoint.row + 1;
            tempPoint.col = currentPoint.col;
            if (goban->board[tempPoint.row][tempPoint.col] == color &&
                    !IsSeen(seen, seenSize, tempPoint))
            {
                PushStack(&stack, tempPoint);
                seen[seenSize++] = tempPoint;
            }
        }
        if (currentPoint.col > 0)
        {
            tempPoint.row = currentPoint.row;
            tempPoint.col = currentPoint.col - 1;
            if (goban->board[tempPoint.row][tempPoint.col] == color &&
                    !IsSeen(seen, seenSize, tempPoint))
            {
                PushStack(&stack, tempPoint);
                seen[seenSize++] = tempPoint;
            }
        }
        if (currentPoint.col < gameInfo->boardSize - 1)
        {
            tempPoint.row = currentPoint.row;
            tempPoint.col = currentPoint.col + 1;
            if (goban->board[tempPoint.row][tempPoint.col] == color &&
                    !IsSeen(seen, seenSize, tempPoint))
            {
                PushStack(&stack, tempPoint);
                seen[seenSize++] = tempPoint;
            }
        }
    }
    int i;
    for (i = 0; i < seenSize; ++i)
    {
        goban->board[seen[i].row][seen[i].col] = ' ';
    }
    return seenSize;
}

char FindBelongsTo(Goban* goban, Point start)
{
    GameInfo* gameInfo = GetGameInfo();
    int i;
    for (i  = start.col; i > 0; i--)
    {
        char current = goban->board[start.row][i];
        if (current != ' ')
            return current;
    }
    for (i  = start.col; i < gameInfo->boardSize; i++)
    {
        char current = goban->board[start.row][i];
        if (current != ' ')
            return current;
    }
    for (i  = start.row; i > 0; i--)
    {
        char current = goban->board[i][start.col];
        if (current != ' ')
            return current;
    }
    for (i  = start.row; i < gameInfo->boardSize; i++)
    {
        char current = goban->board[i][start.col];
        if (current != ' ')
            return current;
    }
    return 'x';
}

int ScoreArea(Goban* goban, Point start, char searched[19][19])
{
    Stack stack;
    Point seen[361];
    int seenSize = 0;
    char belongs_to = FindBelongsTo(goban, start);

    GameInfo* gameInfo = GetGameInfo();
    memset(seen, -1, sizeof(Point) * 361);
    ClearStack(&stack);
    PushStack(&stack, start);
    while (StackSize(&stack) > 0)
    {
        Point currentPoint = PopStack(&stack);
        Point tempPoint = currentPoint;
        seen[seenSize++] = currentPoint;

        if (currentPoint.row > 0)
        {
            tempPoint.row = currentPoint.row - 1;
            tempPoint.col = currentPoint.col;
            if (goban->board[tempPoint.row][tempPoint.col] == ' ' &&
                    !IsSeen(seen, seenSize, tempPoint))
            {
                PushStack(&stack, tempPoint);
            }
            else if (goban->board[tempPoint.row][tempPoint.col] != ' ' &&
                    goban->board[tempPoint.row][tempPoint.col] != belongs_to)
                belongs_to = 'x';
        }
        if (currentPoint.row < gameInfo->boardSize - 1)
        {
            tempPoint.row = currentPoint.row + 1;
            tempPoint.col = currentPoint.col;
            if (goban->board[tempPoint.row][tempPoint.col] == ' ' &&
                    !IsSeen(seen, seenSize, tempPoint))
            {
                PushStack(&stack, tempPoint);
            }
            else if (goban->board[tempPoint.row][tempPoint.col] != ' ' &&
                    goban->board[tempPoint.row][tempPoint.col] != belongs_to)
                belongs_to = 'x';
        }
        if (currentPoint.col > 0)
        {
            tempPoint.row = currentPoint.row;
            tempPoint.col = currentPoint.col - 1;
            if (goban->board[tempPoint.row][tempPoint.col] == ' ' &&
                    !IsSeen(seen, seenSize, tempPoint))
            {
                PushStack(&stack, tempPoint);
            }
            else if (goban->board[tempPoint.row][tempPoint.col] != ' ' &&
                    goban->board[tempPoint.row][tempPoint.col] != belongs_to)
                belongs_to = 'x';
        }
        if (currentPoint.col < gameInfo->boardSize - 1)
        {
            tempPoint.row = currentPoint.row;
            tempPoint.col = currentPoint.col + 1;
            if (goban->board[tempPoint.row][tempPoint.col] == ' ' &&
                    !IsSeen(seen, seenSize, tempPoint))
            {
                PushStack(&stack, tempPoint);
            }
            else if (goban->board[tempPoint.row][tempPoint.col] != ' ' &&
                    goban->board[tempPoint.row][tempPoint.col] != belongs_to)
                belongs_to = 'x';
        }
    }
    int i;
    for (i = 0; i < seenSize; ++i)
    {
        searched[seen[i].row][seen[i].col] = belongs_to;
    }
    return (belongs_to == 'x') ? 0 : seenSize;
}

int RemoveDeadGroups(Goban* goban, char input[256])
{
    char tokens[256][256];
    int terms = tokenize_command(input, tokens);
    int i;
    for (i = 0; i < terms; ++i)
    {
        Point p;
        if (ValidateInput(goban, &p, tokens[i]))
        {
            char color = goban->board[p.row][p.col];
            int points = RemoveGroup(goban, p);
                if (color == 'w')
                    goban->wpris += points;
                else if (color == 'b')
                    goban->bpris += points;
        }
        else return 0;
    }
    return 1;
}

void UpdateResult(Goban* goban)
{
    int black_score = 0;
    int white_score = 0;
    GameInfo* gameInfo = GetGameInfo();
    ScoreBoard(goban);
    int i, j;
    for (i = 0; i < gameInfo->boardSize; ++i)
    {
        for (j = 0; j < gameInfo->boardSize; ++j)
        {
            if (goban->score[i][j] == 'w')
                white_score++;
            else if (goban->score[i][j] == 'b')
                black_score++;
        }
    }
    white_score -= goban->wpris;
    black_score -= goban->bpris;
    float diff = white_score - black_score;
    diff += gameInfo->komi;
    int idx = 0;
    if (diff < 0)
    {
        diff *= -1;
        idx += snprintf(gameInfo->result, 256, "B+");
    }
    else if (diff > 0)
        idx += snprintf(gameInfo->result, 256, "W+");
    snprintf(gameInfo->result + idx, 256, "%.1f", diff);
}

void ScoreBoard(Goban* goban)
{
    GameInfo* gameInfo = GetGameInfo();
    memset(goban->score, ' ', 19 * 19);
    int i, j;
    for (i = 0; i < gameInfo->boardSize; ++i)
    {
        for (j = 0; j < gameInfo->boardSize; ++j) {
            if ((goban->board[i][j] == ' ') && (goban->score[i][j] == ' '))
            {
                Point p;
                p.row = i;
                p.col = j;
                ScoreArea(goban, p, goban->score);
            }
        }
    }
    goban->showscore = 1;
}

// Checks if two Gobans are equal
int IsEqual(Goban* a, Goban* b)
{
    int i, j;
    for (i = 0; i < 19; ++i)
    {
        for (j = 0; j < 19; ++j)
        {
            if (a->board[i][j] != b->board[i][j])
                return 0;
        }
    }
    return 1;
}

// Checks if the current board has appeared before. Used for ko checking
int IsRepeat(Goban* goban)
{
    if (GetHistorySize() == 0)
        return 0;
    GameNode* node = GetViewedNode();
    while (node->mainline_prev != NULL)
    {
        node = node->mainline_prev;
        if (IsEqual(goban, &(node->goban)))
            return 1;
    }
    return 0;
}


/* Validates a move, and if it is valid makes the move*/
int ValidateMove(Goban* goban, Move move)
{
    GameInfo* gameInfo = GetGameInfo();
    if (goban->board[move.p.row][move.p.col] != ' ')
        return 0;
    Goban tempgoban;
    memcpy(&tempgoban, goban, sizeof(Goban));
    tempgoban.board[move.p.row][move.p.col] = move.color;
    tempgoban.lastmove = move;
    tempgoban.color = (move.color == 'b') ? 'w' : 'b';
    char search[19][19];
    memset(search, ' ', 361);
    int i, j;
    for (i = 0; i < gameInfo->boardSize; ++i)
    {
        for (j = 0; j < gameInfo->boardSize; ++j)
        {
            if (search[i][j] == 'x')
                continue;
            if (tempgoban.board[i][j] == tempgoban.color)
            {
                Point p;
                p.row = i;
                p.col = j;
                int captured = 0;
                if (!CountLiberties(&tempgoban, p, search))
                    captured = RemoveGroup(&tempgoban, p);
                if (tempgoban.color == 'w')
                    tempgoban.wpris += captured;
                else if (tempgoban.color == 'b')
                    tempgoban.bpris += captured;
            }
        }
    }
    if (!CountLiberties(&tempgoban, move.p, search) || IsRepeat(&tempgoban))
        return 0;
    else
    {
        if (!AddHistory(&tempgoban))
            return 0;
        memcpy(goban, &tempgoban, sizeof(Goban));
        return 1;
    }
}
