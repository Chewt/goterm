#ifndef DISPLAY_H
#define DISPLAY_H

#include "go.h"
#include "gametree.h"

#define NOTES_LENGTH 512
#define AppendNotes(...) snprintf(GetNotes() + strlen(GetNotes()),\
        NOTES_LENGTH, __VA_ARGS__)
#define WriteNotes(...) snprintf(GetNotes(),\
        NOTES_LENGTH, __VA_ARGS__)
#define AppendComment(n,...) snprintf((n)->comment + strlen((n)->comment),\
        COMMENT_LENGTH, __VA_ARGS__)
#define WriteComment(n,...) snprintf((n)->comment,\
        COMMENT_LENGTH, __VA_ARGS__)

typedef struct 
{
    int centerBoard;
    int showBoard;
    int showInfo;
    int showComments;
} DisplayConfig;

// Is the current window size big enough to print the board
int BoardFitsScreen(Goban* goban);

/* Prints the goban to the screen */
void PrintBoard(Goban* goban);
void PrintBoardw(Goban* goban);
void PrintNotesw(Goban* goban);
void PrintDisplay(Goban* goban);

/* Get buffer to raw notes */
char* GetNotes();
DisplayConfig* GetDisplayConfig();
#endif
