#ifndef GAMEINFO_H
#define GAMEINFO_H

#define NAME_LENGTH 100
#define RESULT_LENGTH 10

typedef struct
{
    char blackName[NAME_LENGTH];
    char whiteName[NAME_LENGTH];
    int boardSize;
    float komi;
    int handicap;
    char result[RESULT_LENGTH];
} GameInfo;

GameInfo* GetGameInfo();

#endif 
