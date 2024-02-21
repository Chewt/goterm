#ifndef DISPLAY_H
#define DISPLAY_H

#include "go.h"

#define NOTES_LENGTH 512
#define AppendNotes(...) snprintf(GetNotes() + strlen(GetNotes()),\
        NOTES_LENGTH, __VA_ARGS__)
#define WriteNotes(...) snprintf(GetNotes(),\
        NOTES_LENGTH, __VA_ARGS__)

// Is the current window size big enough to print the board
int BoardFitsScreen(Goban* goban);

/* Prints the goban to the screen */
void PrintBoard(Goban* goban);
void PrintBoardw(Goban* goban);
void PrintNotesw(Goban* goban);

/* Get buffer to raw notes */
char* GetNotes();

#endif
