#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include "go.h"
#include "stack.h"
#include "commands.h"

char coords[] = {
    'A', 'B', 'C', 'D', 'E', 'F',
    'G', 'H', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T'};

Goban history[500];
int h_counter = 0;
int view_idx = 0;

// Return 0 if input isn't in move format, else returns 1 and populates p
int ValidateInput(Goban* goban, Point* p, char input[256])
{
    char col = input[0];
    int row = input[1] - '0';
    int i;
    for (i = 2; i < 256 && row > 0 && row < goban->size; ++i)
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
    if (row >= 1 && row <= goban->size)
        row = goban->size - row;
    else
        return 0;
    if (col < 0 || col > goban->size)
        return 0;
    p->row = row;
    p->col = col;
    return 1;
}

void AddHistory(Goban* goban)
{
    memcpy(history + h_counter, goban, sizeof(Goban));
    h_counter++;
    view_idx = h_counter - 1;
}

void UndoHistory(Goban* goban, int n)
{
    if (h_counter - n >= 0)
    {
        h_counter -= n;
        view_idx = h_counter - 1;
        memcpy(goban, history + h_counter - 1, sizeof(Goban));
    }
}

void ViewHistory(Goban* goban, int n)
{
    if (HistorySize() == 0)
        return;
    if (n >= HistorySize())
        n = HistorySize() - 1;
    if (n < 0)
        n = 0;
    view_idx = n;
    memcpy(goban, history + n, sizeof(Goban));
}


Goban* GetHistory(int i)
{
    return history + i;
}

int GetViewIndex()
{
    return view_idx;
}
int HistorySize()
{
    return h_counter;
}

void ResetGoban(Goban* goban)
{
    ClearBoard(goban);
    if (goban->notes)
        goban->notes[0] = '\0';
    goban->wpris = 0;
    goban->bpris = 0;
    goban->komi = 6.5;
    goban->handicap = 0;
    goban->size = (goban->size) ? goban->size : 19;
    goban->color = 'b';
    goban->result[0] = '\0';
    strcpy(goban->blackname, "Black");
    strcpy(goban->whitename, "White");
    goban->lastmove.color = 'b';
    goban->lastmove.p.col = -2;
    goban->lastmove.p.row = -2;
    goban->showscore = 0;
    h_counter = 0;
    AddHistory(goban);
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
    int line = (goban->size <= 9) ? 3 : 4;
    switch (numStones)
    {
        case 1:
            goban->board[goban->size / 2][goban->size / 2] = 'b';
            break;
        case 2:
            goban->board[goban->size - line][line - 1] = 'b';
            goban->board[line - 1][goban->size - line] = 'b';
            break;
        case 3:
            goban->board[line - 1][line - 1] = 'b';
            goban->board[goban->size - line][line - 1] = 'b';
            goban->board[line - 1][goban->size - line] = 'b';
            break;
        case 4:
            goban->board[line - 1][line - 1] = 'b';
            goban->board[goban->size - line][line - 1] = 'b';
            goban->board[line - 1][goban->size - line] = 'b';
            goban->board[goban->size - line][goban->size - line] = 'b';
            break;
        case 5:
            goban->board[line - 1][line - 1] = 'b';
            goban->board[goban->size - line][line - 1] = 'b';
            goban->board[line - 1][goban->size - line] = 'b';
            goban->board[goban->size - line][goban->size - line] = 'b';
            goban->board[goban->size / 2][goban->size / 2] = 'b';
            break;
        case 6:
            goban->board[line - 1][line - 1] = 'b';
            goban->board[goban->size - line][line - 1] = 'b';
            goban->board[line - 1][goban->size - line] = 'b';
            goban->board[goban->size - line][goban->size - line] = 'b';
            goban->board[line - 1][goban->size / 2] = 'b';
            goban->board[goban->size - line][goban->size / 2] = 'b';
            break;
        case 7:
            goban->board[line - 1][line - 1] = 'b';
            goban->board[goban->size - line][line - 1] = 'b';
            goban->board[line - 1][goban->size - line] = 'b';
            goban->board[goban->size - line][goban->size - line] = 'b';
            goban->board[line - 1][goban->size / 2] = 'b';
            goban->board[goban->size - line][goban->size / 2] = 'b';
            goban->board[goban->size / 2][goban->size / 2] = 'b';
            break;
        case 8:
            goban->board[line - 1][line - 1] = 'b';
            goban->board[goban->size - line][line - 1] = 'b';
            goban->board[line - 1][goban->size - line] = 'b';
            goban->board[goban->size - line][goban->size - line] = 'b';
            goban->board[line - 1][goban->size / 2] = 'b';
            goban->board[goban->size - line][goban->size / 2] = 'b';
            goban->board[goban->size / 2][line - 1] = 'b';
            goban->board[goban->size / 2][goban->size - line] = 'b';
            break;
        case 9:
            goban->board[line - 1][line - 1] = 'b';
            goban->board[goban->size - line][line - 1] = 'b';
            goban->board[line - 1][goban->size - line] = 'b';
            goban->board[goban->size - line][goban->size - line] = 'b';
            goban->board[line - 1][goban->size / 2] = 'b';
            goban->board[goban->size - line][goban->size / 2] = 'b';
            goban->board[goban->size / 2][line - 1] = 'b';
            goban->board[goban->size / 2][goban->size - line] = 'b';
            goban->board[goban->size / 2][goban->size / 2] = 'b';
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
        if (currentPoint.row < goban->size - 1)
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
        if (currentPoint.col < goban->size - 1)
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
        if (currentPoint.row < goban->size - 1)
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
        if (currentPoint.col < goban->size - 1)
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
        goban->board[seen[i].row][seen[i].col] = ' ';
    }
    return seenSize;
}

char FindBelongsTo(Goban* goban, Point start)
{
    int i;
    for (i  = start.col; i > 0; i--)
    {
        char current = goban->board[start.row][i];
        if (current != ' ')
            return current;
    }
    for (i  = start.col; i < goban->size; i++)
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
    for (i  = start.row; i < goban->size; i++)
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
        if (currentPoint.row < goban->size - 1)
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
        if (currentPoint.col < goban->size - 1)
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
    Goban tempgoban;
    memcpy(&tempgoban, goban, sizeof(Goban));
    int i;
    for (i = 0; i < terms; ++i)
    {
        Point p;
        if (ValidateInput(goban, &p, tokens[i]))
        {
            char color = tempgoban.board[p.row][p.col];
            int points = RemoveGroup(&tempgoban, p);
                if (color == 'w')
                    tempgoban.wpris += points;
                else if (color == 'b')
                    tempgoban.bpris += points;
        }
        else return 0;
    }
    ScoreBoard(&tempgoban);
    PrintBoardw(&tempgoban);
    char resp[COMMAND_LENGTH];
    mvprintw(getcury(stdscr), 0, "Does this look right?[Y/n]\n: ");
    refresh();
    getnstr(resp, 256);
    if (resp[0] == 'n' || resp[0] == 'N')
        return 0;
    memcpy(goban, &tempgoban, sizeof(Goban));
    AddHistory(goban);
    return 1;
}

void PointDiff(Goban* goban, char resp[256])
{
    int black_score = 0;
    int white_score = 0;
    ScoreBoard(goban);
    int i, j;
    for (i = 0; i < goban->size; ++i)
    {
        for (j = 0; j < goban->size; ++j)
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
    diff += goban->komi;
    int idx = 0;
    if (diff < 0)
    {
        diff *= -1;
        idx += snprintf(resp, 256, "B+");
    }
    else if (diff > 0)
        idx += snprintf(resp, 256, "W+");
    snprintf(resp + idx, 256, "%.1f", diff);
    strcpy(goban->result, resp);
}

void ScoreBoard(Goban* goban)
{
    memset(goban->score, ' ', 19 * 19);
    int i, j;
    for (i = 0; i < goban->size; ++i)
    {
        for (j = 0; j < goban->size; ++j) {
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
    if (h_counter == 0)
        return 0;
    int i;
    for (i = 0; i < h_counter; ++i)
        if (IsEqual(goban, history + i))
            return 1;
    return 0;
}


/* Validates a move, and if it is valid makes the move*/
int ValidateMove(Goban* goban, Move move)
{
    if (goban->board[move.p.row][move.p.col] != ' ')
        return 0;
    Goban tempgoban;
    memcpy(&tempgoban, goban, sizeof(Goban));
    tempgoban.board[move.p.row][move.p.col] = move.color;
    tempgoban.lastmove = move;
    tempgoban.color = (move.color == 'b') ? 'w' : 'b';
    char search[19][19];
    memset(search, 0, 361);
    int i, j;
    for (i = 0; i < goban->size; ++i)
    {
        for (j = 0; j < goban->size; ++j)
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
        memcpy(goban, &tempgoban, sizeof(Goban));
        AddHistory(goban);
        return 1;
    }
}

int BoardFitsScreen(Goban* goban)
{
    int width_needed = goban->size * 4 + 11;
    int height_needed = goban->size * 2 + 2;
    return (getmaxx(stdscr) >= width_needed) &&
           (getmaxy(stdscr) >= height_needed);
}

// Color pair declarations
enum
{
    BLACK_STONE_COLOR = 1,
    WHITE_STONE_COLOR = 2,
    LAST_STONE_COLOR = 3,
};

// Print board to ncurses screen
void PrintBoardw(Goban* goban)
{
    start_color();
    init_pair(BLACK_STONE_COLOR, COLOR_BLACK, COLOR_YELLOW);
    init_pair(WHITE_STONE_COLOR, COLOR_WHITE, COLOR_YELLOW);
    init_pair(LAST_STONE_COLOR, COLOR_CYAN, COLOR_YELLOW);
    int x_pos = 0;
    int y_pos = 0;

    // Determine where the board should be placed on screen
    int width_needed = goban->size * 4 + 11;
    int start_xpos = 0;
    int max_x = getmaxx(stdscr);
    if ((max_x / 2) >= (width_needed / 2))
        start_xpos = (max_x / 2) - (width_needed / 2);

    clear(); // Set screen to blank

    // Print empty board
    attron(COLOR_PAIR(BLACK_STONE_COLOR));
    int i;
    //    Background color
    for (i = 0; i < (goban->size * 2) + 1; ++i) 
        mvprintw(i, start_xpos, "   %*s", goban->size * 4, ""); // padding abuse
    x_pos = start_xpos + 3;
    y_pos = 1;
    move(y_pos, x_pos);
    //    Top grid
    addch(ACS_ULCORNER);
    addch(ACS_HLINE);
    addch(ACS_HLINE);
    addch(ACS_HLINE);
    for (i = 0; i < goban->size - 2; ++i)
    {
        addch(ACS_TTEE);
        addch(ACS_HLINE);
        addch(ACS_HLINE);
        addch(ACS_HLINE);
    }
    addch(ACS_URCORNER);
    //    Middle Grid
    y_pos++;
    move(y_pos, x_pos);
    int j;
    for (i = 0; i < goban->size - 2; ++i)
    {
        for (j = 0; j < goban->size; ++j)
        {
            addch(ACS_VLINE);
            printw("   ");
        }
        move(++y_pos, x_pos);
        addch(ACS_LTEE);
        addch(ACS_HLINE);
        addch(ACS_HLINE);
        addch(ACS_HLINE);
        for (j = 0; j < goban->size - 2; ++j)
        {
            addch(ACS_PLUS);
            addch(ACS_HLINE);
            addch(ACS_HLINE);
            addch(ACS_HLINE);
        }
        addch(ACS_RTEE);
        move(++y_pos, x_pos);
    }
    for (i = 0; i < goban->size; ++i)
    {
        addch(ACS_VLINE);
        printw("   ");
    }
    //    Bottom Grid
    move(++y_pos, x_pos);
    addch(ACS_LLCORNER);
    addch(ACS_HLINE);
    addch(ACS_HLINE);
    addch(ACS_HLINE);
    for (i = 0; i < goban->size - 2; ++i)
    {
        addch(ACS_BTEE);
        addch(ACS_HLINE);
        addch(ACS_HLINE);
        addch(ACS_HLINE);
    }
    addch(ACS_LRCORNER);

    // Star points or showing score on the board
    const char STARPOINT[] = "\u254b"; // ╋
    if (goban->showscore)
    {
        for (i = 0; i < goban->size; ++i)
        {
            for (j = 0; j < goban->size; ++j)
            {
                if (goban->score[i][j] == 'b' || goban->score[i][j] == 'w')
                {
                    move((i * 2) + 1, (j * 4) + 3 + start_xpos);
                    if (goban->score[i][j] == 'w')
                    {
                        attroff(COLOR_PAIR(BLACK_STONE_COLOR));
                        attrset(COLOR_PAIR(WHITE_STONE_COLOR));
                        attron(A_BOLD);
                    }
                    addstr(STARPOINT); 
                    attrset(COLOR_PAIR(BLACK_STONE_COLOR));
                }
            }
        }
        goban->showscore = 0;
    }
    else if (goban->size == 19)
    {
        mvaddstr(3 * 2 + 1, 3   * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(3 * 2 + 1, 9   * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(3 * 2 + 1, 15  * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(9 * 2 + 1, 3   * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(9 * 2 + 1, 9   * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(9 * 2 + 1, 15  * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(15 * 2 + 1, 3  * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(15 * 2 + 1, 9  * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(15 * 2 + 1, 15 * 4 + 3 + start_xpos, STARPOINT);
    }
    else if (goban->size == 13)
    {
        mvaddstr(3 * 2 + 1, 3 * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(3 * 2 + 1, 6 * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(3 * 2 + 1, 9 * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(6 * 2 + 1, 3 * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(6 * 2 + 1, 6 * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(6 * 2 + 1, 9 * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(9 * 2 + 1, 3 * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(9 * 2 + 1, 6 * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(9 * 2 + 1, 9 * 4 + 3 + start_xpos, STARPOINT);
    }
    else if (goban->size == 9)
    {
        mvaddstr(2 * 2 + 1, 2 * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(2 * 2 + 1, 6 * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(6 * 2 + 1, 2 * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(6 * 2 + 1, 6 * 4 + 3 + start_xpos, STARPOINT);
    }

    // Print coordinates
    move(0, 3 + start_xpos); 
    for (i = 0; i < goban->size; ++i)
        printw("%c   ", coords[i]); 
    move((goban->size * 2), 3 + start_xpos);
    for (i = 0; i < goban->size; ++i)
        printw("%c   ", coords[i]);
    x_pos = start_xpos;
    y_pos = 1;
    for (i = 0; i < goban->size ; ++i)
    {
        move(y_pos, x_pos);
        printw("%2d", goban->size - i);
        move(y_pos, x_pos + 3 + ((goban->size - 1) * 4) + 2);
        printw("%2d", goban->size - i);
        y_pos += 2;
    }

    // Game info
    attroff(COLOR_PAIR(BLACK_STONE_COLOR));
    mvprintw(0, goban->size * 4 + 4 + start_xpos, "B[%s]: %d", goban->blackname, goban->bpris);
    mvprintw(1, goban->size * 4 + 4 + start_xpos, "W[%s]: %d", goban->whitename, goban->wpris);
    if (goban->result[0] != '\0')
        mvprintw(2, goban->size * 4 + 4 + start_xpos, "Result: %s", goban->result);
    char lastmove[10] = { 0 };
    if (HistorySize() > 1)
    {
        int idx = snprintf(lastmove, 10, "%d. ", GetViewIndex());
        if (goban->lastmove.p.col == -1)
            snprintf(lastmove + idx, 10, "Pass");
        else
        {
            snprintf(lastmove + idx, 10, "%c%d",
                    coords[goban->lastmove.p.col],
                    goban->size - goban->lastmove.p.row);
        }
    }
    mvprintw(4, goban->size * 4 + 4 + start_xpos, "%s", lastmove);
    attron(COLOR_PAIR(BLACK_STONE_COLOR));

    // Place stones
    for (i = 0; i < goban->size; ++i)
    {
        for (j = 0; j < goban->size; ++j)
        {
            if (goban->board[i][j] != ' ')
            {
                move((i * 2) + 1, (j * 4) + 2 + start_xpos);
                if (goban->board[i][j] == 'w')
                {
                    attroff(COLOR_PAIR(BLACK_STONE_COLOR));
                    attrset(COLOR_PAIR(WHITE_STONE_COLOR));
                    attron(A_BOLD);
                }
                addstr("\u2588\u2588"); // Block char

                if (goban->lastmove.p.col == j && goban->lastmove.p.row == i)
                    attrset(COLOR_PAIR(LAST_STONE_COLOR));
                addstr("\u2588");
                attrset(COLOR_PAIR(BLACK_STONE_COLOR));
            }
        }
    }
    move(goban->size * 2 + 1, start_xpos);
    attroff(COLOR_PAIR(BLACK_STONE_COLOR));
}

void PrintNotesw(Goban* goban)
{
    // Count how many lines the notes have
    char c;
    int i = 0;
    int linecount = 0;
    while ((c = goban->notes[i++]) != '\0')
    {
        if (c == '\n')
            linecount++;
    }

    // Print notes and adjust starting y for notes
    int x, y;
    x = 0;
    y = getcury(stdscr);
    if ((getmaxy(stdscr) - y) <= linecount)
        y -= linecount;
    i = 0;
    while ((c = goban->notes[i++]) != '\0')
    {
        if (c == '\n')
            move(++y, x = 0);
        else
            mvaddch(y, x++, c);
    }

    // Reset notes to empty
    goban->notes[0] = '\0';
}

// Print board to screen
void PrintBoard(Goban* goban)
{
    printf("\e[2J\e[H"); // Clear Screen and position cursor top left
    int i, j;
    printf("\e[30;43m ");
    for (i = 0; i < goban->size; ++i)
        printf("  %c ", coords[i]);
    printf("  \e[0m B: %d\n", goban->bpris);
    for (i = 0; i < (2 * goban->size) - 1; ++i)
    {
        for (j = 0; j < goban->size; ++j)
        {
            printf("\e[30;43m"); // Reset terminal colors
            if (goban->board[i/2][j] == ' ' || (i & 0x1))
            {
                if (i & 0x1)
                {
                    if (j == 0)
                        printf("  ");
                    printf(" \u2502"); // │
                    if (j == goban->size - 1)
                        printf("   \e[0m\n");
                    else
                        printf("  ");
                }
                else if (i == 0)
                {
                    if (j == 0){
                        printf("%2d \e[30;43m", goban->size);
                        if (goban->showscore && ((goban->score[i][j] == 'w') ||
                                    (goban->score[i/2][j] == 'b'))) 
                        {
                            if (goban->score[i/2][j] == 'w')
                                printf("\e[97;43m");
                            printf("\u254b"); // ╋
                            printf("\e[30;43m");
                        }
                        else
                            printf("\u250c"); // ┌
                        printf("\u2500\u2500"); // ──
                    }
                    else if (j == goban->size - 1){ // 
                        printf("\u2500"); // ─
                        if (goban->showscore && ((goban->score[i/2][j] == 'w') ||
                                    (goban->score[i/2][j] == 'b'))) 
                        {
                            if (goban->score[i][j] == 'w')
                                printf("\e[97;43m");
                            printf("\u254b"); // ╋
                            printf("\e[30;43m");
                        }
                        else
                            printf("\u2510"); // ┐
                        printf(" %2d\e[0m W: %d\n", goban->size,
                                goban->wpris);
                    }
                    else
                    {
                        printf("\u2500"); // ─
                        if (goban->showscore && ((goban->score[i/2][j] == 'w') ||
                                    (goban->score[i/2][j] == 'b'))) 
                        {
                            if (goban->score[i/2][j] == 'w')
                                printf("\e[97;43m");
                            printf("\u254b"); // ╋
                            printf("\e[30;43m");
                        }
                        else
                            printf("\u252c"); // ┬
                        printf("\u2500\u2500"); // ──
                    }
                }
                else if (i == (2 * goban->size) - 2)
                {
                    if (j == 0) {

                        printf(" 1\e[30;43m ");
                        if (goban->showscore && ((goban->score[i/2][j] == 'w') ||
                                    (goban->score[i/2][j] == 'b'))) 
                        {
                            if (goban->score[i/2][j] == 'w')
                                printf("\e[97;43m");
                            printf("\u254b"); // ╋
                            printf("\e[30;43m");
                        }
                        else
                            printf("\u2514"); // └
                        printf("\u2500\u2500"); //──
                    }
                    else if (j == goban->size - 1)
                    {
                        printf("\u2500"); // ─
                        if (goban->showscore && ((goban->score[i/2][j] == 'w') ||
                                    (goban->score[i/2][j] == 'b'))) 
                        {
                            if (goban->score[i/2][j] == 'w')
                                printf("\e[97;43m");
                            printf("\u254b"); // ╋
                            printf("\e[30;43m");
                        }
                        else
                            printf("\u2518"); // ┘
                        printf("  1\e[0m\n"); 
                        i++;
                    }
                    else
                    {
                        printf("\u2500"); // ─
                        if (goban->showscore && ((goban->score[i/2][j] == 'w') ||
                                    (goban->score[i/2][j] == 'b'))) 
                        {
                            if (goban->score[i/2][j] == 'w')
                                printf("\e[97;43m");
                            printf("\u254b"); // ╋
                            printf("\e[30;43m");
                        }
                        else
                            printf("\u2534"); // ┴
                        printf("\u2500\u2500"); // ──
                    }
                }
                else
                {
                    if (j == 0)
                    {
                        printf("%2d \e[30;43m", goban->size - (i / 2));
                        if (goban->showscore && ((goban->score[i/2][j] == 'w') ||
                                    (goban->score[i/2][j] == 'b'))) 
                        {
                            if (goban->score[i/2][j] == 'w')
                                printf("\e[97;43m");
                            printf("\u254b"); // ╋
                            printf("\e[30;43m");
                        }
                        else
                            printf("\u251c"); // ├
                        printf("\u2500\u2500"); // ──
                    }
                    else if (j == goban->size - 1)
                    {
                        printf("\u2500"); // ─
                        if (goban->showscore && ((goban->score[i/2][j] == 'w') ||
                                    (goban->score[i/2][j] == 'b'))) 
                        {
                            if (goban->score[i/2][j] == 'w')
                                printf("\e[97;43m");
                            printf("\u254b"); // ╋
                            printf("\e[30;43m");
                        }
                        else
                            printf("\u2524"); // ┤
                        printf(" %2d\e[0m", goban->size - (i / 2));
                        if (i == 2)
                        {
                            char lastmove[5] = { 0 };
                            if (HistorySize() >= 1)
                            {
                                if (goban->lastmove.p.col == -1)
                                    snprintf(lastmove, 5, "Pass");
                                else
                                {
                                    snprintf(lastmove, 5, "%c%d",
                                            coords[goban->lastmove.p.col],
                                            goban->size - goban->lastmove.p.row);
                                }
                            }
                            printf(" %s", lastmove);
                        }
                        printf("\n");
                    }
                    else
                    {
                        printf("\u2500"); // ─
                        if (goban->showscore == 0)
                        {
                            if (goban->size == 19 &&
                                    (i == 6 || i == 18 || i == 30) &&
                                    (j == 3 || j == 9  || j == 15))
                                printf("\u254b"); // ╋
                            else if (goban->size == 13 && 
                                    (i == 6 || i == 12 || i == 18) &&
                                    (j == 3 || j == 6  || j == 9 ))
                                printf("\u254b"); // ╋
                            else if (goban->size == 9 && 
                                    (i == 4 || i == 12) &&
                                    (j == 2 || j == 6))
                                printf("\u254b"); // ╋
                            else printf("\u253c"); // ┼
                        }
                        else
                        {
                            if ((goban->score[i/2][j] == 'w') ||
                                    (goban->score[i/2][j] == 'b'))
                            {
                                if (goban->score[i/2][j] == 'w')
                                    printf("\e[97;43m");
                                printf("\u254b"); // ╋
                                printf("\e[30;43m");
                            }
                            else
                                printf("\u253c"); // ┼
                        }
                        printf("\u2500\u2500"); // ──
                    }
                }
            }
            else
            {
                if (j == 0)
                    printf("%2d", goban->size - (i/2));
                if (goban->board[i/2][j] == 'w')
                    printf("\e[97;43m");
                printf("\u2588\u2588"); // █
                if (goban->lastmove.p.col == j && goban->lastmove.p.row == i/2)
                    printf("\e[36m");
                printf("\u2588");
                if (j == goban->size - 1)
                {
                    printf("\e[30;43m%2d\e[0m ", goban->size - (i/2));
                    if (i == 0)
                        printf("W: %d", goban->wpris);
                    printf("\n");
                }
                else
                    printf("\e[0m\e[30;43m\u2500");
            }
        }
    }
    printf("\e[30;43m ");
    for (i = 0; i < goban->size; ++i)
        printf("  %c ", coords[i]);
    goban->showscore = 0;
    printf("  \e[0m\n");
}
