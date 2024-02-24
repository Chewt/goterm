#include <string.h>
#include "gameinfo.h"

GameInfo gameInfo;

GameInfo* GetGameInfo()
{
    return &gameInfo;
}

void SetGameInfoDefaults()
{
    gameInfo.can_edit = 1;
    gameInfo.komi = 6.5f;
    gameInfo.boardSize = 19;
    gameInfo.handicap = 0;
    gameInfo.result[0] = '\0';
    strcpy(gameInfo.blackName, "Black");
    strcpy(gameInfo.whiteName, "White");
}
