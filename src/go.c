#include <stdio.h>
#include <string.h>
#include "go.h"
#include "stack.h"

Goban history[500];
int h_counter = 0;

void AddHistory(Goban* goban)
{
    //history[h_counter] = *goban;
    memcpy(history + h_counter, goban, sizeof(Goban));
    h_counter++;
}

void UndoHistory(Goban* goban)
{
    h_counter--;
    memcpy(goban, history + h_counter, sizeof(Goban));
    //*goban = history[h_counter];
}

void ResetGoban(Goban* goban)
{
    ClearBoard(goban);
    goban->hasko = 0;
    goban->wpris = 0;
    goban->bpris = 0;
}

void ClearBoard(Goban* goban)
{
    int i, j;
    for (i = 0; i < 19; ++i)
        for (j = 0; j < 19; ++j)
        {
            goban->board[i][j] = ' ';
        }
}

int HasLiberties(Goban* goban, Point point)
{
    char color = goban->board[point.row][point.col];
    int liberties = 0;
    if (point.row > 0)
    {
        if (goban->board[point.row - 1][point.col] == ' ')
            liberties++;
    }
    if (point.row < 18)
    {
        if (goban->board[point.row + 1][point.col] == ' ')
            liberties++;
    }
    if (point.col > 0)
    {
        if (goban->board[point.row][point.col - 1] == ' ')
            liberties++;
    }
    if (point.col < 18)
    {
        if (goban->board[point.row][point.col + 1] == ' ')
            liberties++;
    }
    return liberties;
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

int SearchGroup(Goban* goban, Point start, char searched[19][19])
{
    Stack stack;
    Point seen[361];
    int seenSize = 0;
    int liberties = 0;
    char color = goban->board[start.row][start.col];

    memset(seen, ' ', sizeof(Point) * 361);
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
        if (currentPoint.row < 18)
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
        if (currentPoint.col < 18)
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
    for (i = 0; i < seenSize; ++i)
    {
        if (liberties > 0)
        {
            searched[seen[i].row][seen[i].col] = 'x';
        }
        else
        {
            goban->board[seen[i].row][seen[i].col] = ' ';
        }
    }
    return (liberties) ? 0 : seenSize;
}

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

int IsRepeat(Goban* goban)
{
    if (h_counter == 0)
        return 0;
    int i;
    for (i = 0; i < h_counter; ++i)
        if (IsEqual(goban, history + i))
            return 1;
    return 0;
}

int ValidateMove(Goban* goban, Point move)
{
    Goban tempgoban;
    memcpy(&tempgoban, goban, sizeof(Goban));
    tempgoban.board[move.row][move.col] = goban->color;
    tempgoban.lastmove = move;
    tempgoban.color = (tempgoban.color == 'b') ? 'w' : 'b';
    char search[19][19];
    memset(search, 0, 361);
    int i, j;
    for (i = 0; i < 19; ++i)
    {
        for (j = 0; j < 19; ++j)
        {
            if (search[i][j] == 'x')
                continue;
            if (tempgoban.board[i][j] == tempgoban.color)
            {
                Point p;
                p.row = i;
                p.col = j;
                int captured = SearchGroup(&tempgoban, p, search);
                if (tempgoban.color == 'w')
                    tempgoban.wpris += captured;
                else if (tempgoban.color == 'b')
                    tempgoban.bpris += captured;
            }
        }
    }
    if (!HasLiberties(&tempgoban, move) || IsRepeat(&tempgoban))
        return 0;
    else
    {
        AddHistory(goban);
        memcpy(goban, &tempgoban, sizeof(Goban));
        return 1;
    }
}

void PrintBoard(Goban* goban)
{
    int i, j;
    printf("\e[30;43m ");
    for (i = 0; i < 19; ++i)
        printf("  %c ", 'A' + i);
    printf("  \e[0m\n");
    for (i = 0; i < 37; ++i)
    {
        for (j = 0; j < 19; ++j)
        {
            printf("\e[30;43m");
            if (goban->board[i/2][j] == ' ' || (i & 0x1))
            {
                if (i & 0x1)
                {
                    if (j == 0)
                        printf("  ");
                    printf(" \u2502");
                    if (j == 18)
                        printf("   \e[0m\n");
                    else
                        printf("  ");
                }
                else if (i == 0)
                {
                    switch (j)
                    {
                        case 0:
                            printf("19 \e[30;43m\u250c\u2500\u2500");
                            break;
                        case 18:
                            printf("\u2500\u2510 19\e[0m\n");
                            break;
                        default:
                            printf("\u2500\u252c\u2500\u2500");
                            break;
                    }
                }
                else if (i == 36)
                {
                    switch (j)
                    {
                        case 0:
                            printf(" 1\e[30;43m \u2514\u2500\u2500");
                            break;
                        case 18:
                            printf("\u2500\u2518  1\e[0m\n");
                            i++;
                            break;
                        default:
                            printf("\u2500\u2534\u2500\u2500");
                            break;
                    }
                }
                else
                {
                    switch (j)
                    {

                        case 0:
                            printf("%2d \e[30;43m\u251c\u2500\u2500", 19 - (i/2));
                            break;
                        case 18:
                            printf("\u2500\u2524 %2d\e[0m\n", 19 - (i/2));
                            break;
                        default:
                            printf("\u2500");
                            if ((i == 6 || i == 18 || i == 30) 
                              && (j == 3 || j == 9 || j == 15))
                                printf("\u254b");
                            else
                                printf("\u253c");
                            printf("\u2500\u2500");
                            break;
                    }
                }
            }
            else
            {
                if (j == 0)
                    printf("%2d", 19 - (i/2));
                if (goban->board[i/2][j] == 'w')
                    printf("\e[97;43m");
                printf("\u2588\u2588\u2588");
                if (j == 18)
                    printf("\e[30;43m%2d\e[0m\n", 19 - (i/2));
                else
                    printf("\e[30;43m\u2500");
            }
        }
    }
    printf("\e[30;43m ");
    for (i = 0; i < 19; ++i)
        printf("  %c ", 'A' + i);
    printf("  \e[0m\n");
}
