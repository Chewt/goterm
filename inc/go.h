#ifndef GO_H
#define GO_H

#define NOTES_LENGTH 512

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
    char* notes;
    char board[19][19];
    char score[19][19];
    int showscore;
    int size;
    Move lastmove;
    char color;
    int wpris;
    int bpris;
    float komi;
    char result[8];
} Goban;

void ResetGoban(Goban* goban);
void ClearBoard(Goban* goban);
void AddHistory(Goban* goban);
void UndoHistory(Goban* goban);
void ScoreBoard(Goban* goban);
int RemoveDeadGroups(Goban* goban, char input[256]);
void PointDiff(Goban* goban, char resp[256]);
int HistorySize();
Goban* GetHistory(int i);

int ValidateInput(Goban* goban, Point* p, char input[256]);
int ValidateMove(Goban* goban, Point move);

/* Prints the goban to the screen */
void PrintBoard(Goban* goban);
void PrintBoardw(Goban* goban);

#endif
