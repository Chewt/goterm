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
    char whitename[100];
    char blackname[100];
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
    int handicap;
    char result[8];
} Goban;

void ResetGoban(Goban* goban);
void ClearBoard(Goban* goban);
void AddHistory(Goban* goban);
void UndoHistory(Goban* goban, int n);
void ViewHistory(Goban* goban, int n);
void ScoreBoard(Goban* goban);
void SetHandicap(Goban* goban, int numStones);
int RemoveDeadGroups(Goban* goban, char input[256]);
void PointDiff(Goban* goban, char resp[256]);
int HistorySize();
int GetViewIndex();
Goban* GetHistory(int i);

int ValidateInput(Goban* goban, Point* p, char input[256]);
int ValidateMove(Goban* goban, Move move);

// Is the current window size big enough to print the board
int BoardFitsScreen(Goban* goban);

/* Prints the goban to the screen */
void PrintBoard(Goban* goban);
void PrintBoardw(Goban* goban);
void PrintNotesw(Goban* goban);

#endif
