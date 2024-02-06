#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#include "sgf.h"
#include "go.h"

char* CreateSGF()
{
    if (HistorySize() <= 1)
        return NULL;
    time_t rawtime = time(NULL);
    struct tm* ptm = localtime(&rawtime);

    char* sgf = malloc(4096);
    int index = 0;
    
    Goban* current = GetHistory(0);
    Goban* last = GetHistory(HistorySize() - 1);

#ifdef VERSION
    const char* version = VERSION;
#elif 
    const char* version = "debug";
#endif

    index = sprintf(sgf + index, "(;FF[4]GM[1]SZ[%d]AP[Goterm:%s]\n\n", 
            last->size, version);
    index += sprintf(sgf + index, "PB[%s]\n", last->blackname);
    index += sprintf(sgf + index, "PW[%s]\n", last->whitename);
    index += sprintf(sgf + index, "DT[");
    index += strftime(sgf + index, 11, "%F", ptm);
    index += sprintf(sgf + index, "]\n");
    index += sprintf(sgf + index, "KM[");
    index += sprintf(sgf + index, "%.1f", last->komi);
    index += sprintf(sgf + index, "]\n");
    index += sprintf(sgf + index, "HA[%d]\n", last->handicap);
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

char* ReadSGFFile(char* filename)
{
    // Open file
    FILE* f = fopen(filename, "r");

    // Get size of file
    fseek(f, 0, SEEK_END);
    int f_size = ftell(f);
    rewind(f);

    // Make buffer to store file contents
    char* sgf = malloc(f_size);

    // Read contents into buffer
    int i = 0;
    char c;
    while ((c = fgetc(f)) != EOF)
    {
        if (c != '\n')
            sgf[i++] = c;
    }

    fclose(f);
    return sgf;
}

void LoadSGF(Goban* goban, char* sgf)
{
    // Start from clean state
    ResetGoban(goban);
    char* token;
    char* save_ptr_outer;
    token = strtok_r(sgf, ";", &save_ptr_outer); // First token always '(' 

    // Get metadata
    token = strtok_r(NULL, ";", &save_ptr_outer);
    char* meta_data_save_ptr;
    char* md_token;
    md_token = strtok_r(token, "[]", &meta_data_save_ptr);
    do 
    {
        if (md_token[0] == 'P' && md_token[1] == 'W')
        {
            md_token = strtok_r(NULL, "[]", &meta_data_save_ptr);
            bzero(goban->whitename, 100);
            strncpy(goban->whitename, md_token, 99);
        }
        else if (md_token[0] == 'P' && md_token[1] == 'B')
        {
            md_token = strtok_r(NULL, "[]", &meta_data_save_ptr);
            bzero(goban->blackname, 100);
            strncpy(goban->blackname, md_token, 99);
        }
        else if (md_token[0] == 'K' && md_token[1] == 'M')
        {
            md_token = strtok_r(NULL, "[]", &meta_data_save_ptr);
            goban->komi = strtof(md_token, NULL);
        }
        else if (md_token[0] == 'S' && md_token[1] == 'Z')
        {
            md_token = strtok_r(NULL, "[]", &meta_data_save_ptr);
            goban->size = strtod(md_token, NULL);
        }
        else if (md_token[0] == 'H' && md_token[1] == 'A')
        {
            md_token = strtok_r(NULL, "[]", &meta_data_save_ptr);
            SetHandicap(goban, strtod(md_token, NULL));
        }
        else if (md_token[0] == 'R' && md_token[1] == 'E')
        {
            md_token = strtok_r(NULL, "[]", &meta_data_save_ptr);
            bzero(goban->result, 10);
            strncpy(goban->result, md_token, 9);
        }
    } while ((md_token = strtok_r(NULL, "[]", &meta_data_save_ptr)) != NULL);

    // Read Moves
    while ((token = strtok_r(NULL, ";", &save_ptr_outer)) != NULL)
    {
        Move m;
        m.color = (token[0] == 'B') ? 'b' : 'w';
        if ((token[1] == '[') && (token[2] == ']')) // Pass
        {
            goban->lastmove.p.col = -1;
            goban->lastmove.p.row = -1;
            goban->lastmove.color = m.color;
            goban->color = (goban->color == 'b') ? 'w' : 'b';
            AddHistory(goban);
        }
        else // Real move
        {
            m.p.col = token[2] - 'a';
            m.p.row = token[3] - 'a';
            ValidateMove(goban, m);
        }
    }
    sprintf(goban->notes, "Loaded game from sgf...\n");
}
