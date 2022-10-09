#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void clearBoard(char board[19][19])
{
    int i, j;
    for (i = 0; i < 19; ++i)
        for (j = 0; j < 19; ++j)
            board[i][j] = ' ';
}

void printBoard(char board[19][19])
{
    int i, j;
    printf("\e[30;43m ");
    for (i = 0; i < 19; ++i)
        printf("  %c ", 'A' + i);
    printf(" \e[0m\n");
    for (i = 0; i < 37; ++i)
    {
        for (j = 0; j < 19; ++j)
        {
            printf("\e[30;43m");
            if (board[i/2][j] == ' ' || (i & 0x1))
            {
                if (i & 0x1)
                {
                    if (j == 0)
                        printf("  ");
                    printf(" \u2502");
                    if (j == 18)
                        printf("  \e[0m\n");
                    else
                        printf("  ");
                }
                else if (i == 0)
                {
                    switch (j)
                    {
                        case 0:
                            printf("19 \e[30;43m\u250c\u2500\u2500");
                            break;
                        case 18:
                            printf("\u2500\u2510  \e[0m\n");
                            break;
                        default:
                            printf("\u2500\u252c\u2500\u2500");
                            break;
                    }
                }
                else if (i == 36)
                {
                    switch (j)
                    {
                        case 0:
                            printf(" 1\e[30;43m \u2514\u2500\u2500");
                            break;
                        case 18:
                            printf("\u2500\u2518  \e[0m\n");
                            i++;
                            break;
                        default:
                            printf("\u2500\u2534\u2500\u2500");
                            break;
                    }
                }
                else
                {
                    switch (j)
                    {

                        case 0:
                            printf("%2d \e[30;43m\u251c\u2500\u2500", 19 - (i/2));
                            break;
                        case 18:
                            printf("\u2500\u2524  \e[0m\n");
                            break;
                        default:
                            printf("\u2500");
                            if ((i == 6 || i == 18 || i == 30) 
                              && (j == 3 || j == 9 || j == 15))
                                printf("\u254b");
                            else
                                printf("\u253c");
                            printf("\u2500\u2500");
                            break;
                    }
                }
            }
            else
            {
                if (j == 0)
                    printf("%2d", 19 - (i/2));
                if (board[i/2][j] == 'w')
                    printf("\e[97;43m");
                printf("\u2588\u2588\u2588");
                if (j == 18)
                    printf("\e[30;43m \e[0m\n");
                else
                    printf("\e[30;43m\u2500");
            }
        }
    }
    printf("\e[30;43m ");
    for (i = 0; i < 19; ++i)
        printf("    ");
    printf(" \e[0m\n");
}

int main()
{
    srand(time(NULL));
    char board[19][19];
    clearBoard(board);
    printBoard(board);
    char move = 'b';
    while (1)
    {
        char input[256];
        fgets(input, 256, stdin);
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
        board[row][col] = move;
        move = (move == 'b') ? 'w' : 'b';
        printBoard(board);
    }
    return 0;
}
