#include <string.h>
#include <ncurses.h>
#include <stdio.h>
#include "commands.h"

struct Keybind
{
    char* keys;
    char command[COMMAND_LENGTH];
};

struct Keybind keybinds[] = {
    {"n", "next"},
    {"b", "back"},
    {"N", "jump next"},
    {"B", "jump back"},
    {"J", "jump down"},
    {"K", "jump up"},
    { 0 }
};

int ProcessKeys(Goban* goban, char* command)
{
    noecho();

    *command = '\0';
    int key = getch();
    if (key == ':')
    {
        echo();
        mvprintw(getcury(stdscr), 0, ":");
        refresh();
        getnstr(command, COMMAND_LENGTH);
        command[strcspn(command, "\n")] = 0;
        noecho();
        return 1;
    }
    int i = 0;
    while (keybinds[i].keys != NULL)
    {
        if (strchr(keybinds[i].keys, key))
        {
            snprintf(command, COMMAND_LENGTH, "%s", keybinds[i].command);
            break;
        }
        i++;
    }
    return 1;
}
