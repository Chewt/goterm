#ifndef GAMEINFO_H
#define GAMEINFO_H

#define NAME_LENGTH 100
#define RANK_LENGTH 10
#define RESULT_LENGTH 10

typedef struct
{
    char blackName[NAME_LENGTH];
    char whiteName[NAME_LENGTH];
    char blackRank[RANK_LENGTH];
    char whiteRank[RANK_LENGTH];
    int boardSize;
    float komi;
    int handicap;
    char result[RESULT_LENGTH];
    int can_edit;
} GameInfo;

GameInfo* GetGameInfo();
void SetGameInfoDefaults();

#endif 
