#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "go.h"

int main()
{
    srand(time(NULL));
    Goban goban;
    ResetGoban(&goban);
    PrintBoard(&goban);
    goban.color = 'b';
    while (1)
    {
        char input[256];
        if (!fgets(input, 256, stdin))
            exit(-1);
        char col = input[0];
        int row = input[1] - '0';
        int i;
        for (i = 2; i < 256; ++i)
        {
            if (input[i] == '\n')
                break;
            row *= 10;
            row += input[i] - '0';
        }
        if (col >= 'A' && col <= 'S')
            col += 32;
        if (col >= 'a' && col <= 's')
            col -= 'a';
        else
        {
            printf("Invalid move\n");
            continue;
        }
        if (row >= 1 && row <= 19)
            row = 19 - row;
        Point p;
        p.row = row;
        p.col = col;
        if (!ValidateMove(&goban, p))
            printf("Invalid Move\n");
        PrintBoard(&goban);
    }
    return 0;
}
