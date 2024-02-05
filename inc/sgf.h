#ifndef SGF_H
#define SGF_H 

#include "go.h"

char* CreateSGF();
void LoadSGF(Goban* goban, char* sgf);
char* ReadSGFFile(char* filename);

#endif 
