#ifndef GO_H
#define GO_H

#define INPUT_LENGTH 256

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
    char score[19][19];
    int showscore;
    Move lastmove;
    char color;
    int wpris;
    int bpris;
} Goban;

void ResetGoban(Goban* goban);
void ClearBoard(Goban* goban);
void AddHistory(Goban* goban);
void UndoHistory(Goban* goban, int n);
void ViewHistory(Goban* goban, int n);
void ScoreBoard(Goban* goban);
void SetHandicap(Goban* goban, int numStones);
int RemoveDeadGroups(Goban* goban, char input[INPUT_LENGTH]);
void UpdateResult(Goban* goban);
int HistorySize();
int GetViewIndex();
Goban* GetHistory(int i);

int ValidateInput(Goban* goban, Point* p, char input[INPUT_LENGTH]);
int ValidateMove(Goban* goban, Move move);


#endif
