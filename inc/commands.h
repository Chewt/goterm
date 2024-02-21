#ifndef COMMANDS_H
#define COMMANDS_H
#include "go.h"

#define COMMAND_LENGTH 256
#define SWAP 100
#define MOVE 200

typedef struct
{
    char name[COMMAND_LENGTH];
    void* function;

} Command;

int IsNetworkedCommand(char input[COMMAND_LENGTH]);
int ProcessCommand(Goban* goban, char player, char input[COMMAND_LENGTH]);
int SubmitMove(Goban* goban, char input[COMMAND_LENGTH]);
int tokenize_command(char input[COMMAND_LENGTH], char tokens[][256]);

#endif
