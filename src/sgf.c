#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "sgf.h"
#include "go.h"
#include "gtp.h"

char* CreateSGF(Engine* e)
{
    time_t rawtime = time(NULL);
    struct tm* ptm = localtime(&rawtime);

    char* sgf = malloc(4096);
    int index = 0;
    
    Goban* current = GetHistory(0);
    index = sprintf(sgf + index, "(;FF[4]GM[1]SZ[%d]AP[Goterm:1.0]\n\n", 
            current->size);
    index += sprintf(sgf + index, "PB[%s]\n", current->blackname);
    index += sprintf(sgf + index, "PW[%s]\n", current->whitename);
    index += sprintf(sgf + index, "DT[");
    index += strftime(sgf + index, 11, "%F", ptm);
    index += sprintf(sgf + index, "]\n");
    index += sprintf(sgf + index, "KM[");
    index += sprintf(sgf + index, "%.1f", current->komi);
    index += sprintf(sgf + index, "]\n");
    Goban* last = GetHistory(HistorySize() - 1);
    if (last->result[0] != '\0')
    {
        index += sprintf(sgf + index, "RE[");
        index += sprintf(sgf + index, "%s", last->result);
        index += sprintf(sgf + index, "]\n");
    }
    index += sprintf(sgf + index, "\n");

    int i;
    for (i = 1; i < HistorySize(); ++i)
    {
        current = GetHistory(i);
        Move lm = current->lastmove;
        if (lm.p.col != -1)
        {
            index += sprintf(sgf + index, ";%c[%c%c]",
                    lm.color - 32, 'a' + lm.p.col, 'a' + lm.p.row);
        }
        if (i % 10 == 0)
            index += sprintf(sgf + index, "\n");
    }
    index += sprintf(sgf + index, ")");
    sgf[index] = '\0';
    return sgf;
}
