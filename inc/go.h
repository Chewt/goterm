#ifndef GO_H
#define GO_H

typedef struct
{
    int row;
    int col;
} Point;

typedef struct
{
    Point p;
    char color;
} Move;

typedef struct
{
    char board[19][19];
    int size;
    Move lastmove;
    char color;
    int wpris;
    int bpris;
} Goban;

void ResetGoban(Goban* goban);
void ClearBoard(Goban* goban);
void AddHistory(Goban* goban);
void UndoHistory(Goban* goban);
int HistorySize();
Goban* GetHistory(int i);

int ValidateInput(Goban* goban, Point* p, char input[256]);
int ValidateMove(Goban* goban, Point move);

/* Prints the goban to the screen */
void PrintBoard(Goban* goban);

#endif
