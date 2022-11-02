#ifndef GTP_H
#define GTP_H

#include <stdlib.h>
#include "go.h"


typedef struct
{
    pid_t pid;
    char name[100];
    char version[100];
    int write;
    int read;
} Engine;


void SendProtocolVersion(int fd, int id);
void SendName(int fd, int id);
void SendVersion(int fd, int id);
void SendKnownCommand(int fd, int id);
void SendListCommands(int fd, int id);
void SendQuit(int fd, int id);
void SendBoardsize(int fd, int id, int size);
void SendClearBoard(int fd, int id);
void SendKomi(int fd, int id, float new_komi);
void SendPlay(int fd, int id, Move move);
Point SendGenmove(int fd, int id, char color);

#endif /* end of include guard: GTP_H */
