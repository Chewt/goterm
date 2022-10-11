#ifndef GO_H
#define GO_H

typedef struct
{
    int row;
    int col;
} Point;

typedef struct
{
    char board[19][19];
    char koref[19][19];
    Point lastmove;
    char hasko;
    char color;
    int wpris;
    int bpris;
} Goban;

void ResetGoban(Goban* goban);
void ClearBoard(Goban* goban);

int ValidateMove(Goban* goban, Point move);

/* Prints the goban to the screen */
void PrintBoard(Goban* goban);

#endif
