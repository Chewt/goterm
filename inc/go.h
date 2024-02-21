#ifndef GO_H
#define GO_H

#define NOTES_LENGTH 512
#define INPUT_LENGTH 256

#define AppendNotes(g, ...) snprintf((g)->notes + strlen((g)->notes),\
        NOTES_LENGTH, __VA_ARGS__)
#define WriteNotes(g, ...) snprintf((g)->notes,\
        NOTES_LENGTH, __VA_ARGS__)

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

// Is the current window size big enough to print the board
int BoardFitsScreen(Goban* goban);

/* Prints the goban to the screen */
void PrintBoard(Goban* goban);
void PrintBoardw(Goban* goban);
void PrintNotesw(Goban* goban);

#endif
