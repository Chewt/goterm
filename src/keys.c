#include <string.h>
#include <ncurses.h>
#include <stdio.h>
#include "commands.h"


int ProcessKeys(Goban* goban, char* command)
{
    noecho();

    int key = getch();

    switch (key)
    {
        case 'n':
            snprintf(command, COMMAND_LENGTH, "next");
            break;
        case 'b':
            snprintf(command, COMMAND_LENGTH, "back");
            break;
        case 'N':
            snprintf(command, COMMAND_LENGTH, "jump next");
            break;
        case 'B':
            snprintf(command, COMMAND_LENGTH, "jump back");
            break;
        case 'J':
            snprintf(command, COMMAND_LENGTH, "jump down");
            break;
        case 'K':
            snprintf(command, COMMAND_LENGTH, "jump up");
            break;
        case ':':
            {
                echo();
                mvprintw(getcury(stdscr), 0, ":");
                refresh();
                getnstr(command, COMMAND_LENGTH);
                command[strcspn(command, "\n")] = 0;
                noecho();
            }
    }
    return 1;
}
