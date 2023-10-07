#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "gtp.h"
#include "dynarray.h"


#define ENGINE_READ  to_engine[0]
#define ENGINE_WRITE from_engine[1]
#define CONTROLLER_READ  from_engine[0]
#define CONTROLLER_WRITE to_engine[1]

int ValidateResponse(char* resp, int id)
{
    if (resp[0] != '=')
        return 0;
    int read_id = 0;
    int i = 1;
    char c = resp[i];
    while (c <= '9' && c >= '0')
    {
        read_id *= 10;
        read_id += c - '0';
        i++;
        c = resp[i];
    }
    if (read_id == id)
        return 1;
    return 0;
}

char* GetMessage(int fd)
{
    struct dynarray* all_bytes = dynarray_create();
    int bytes_read = 0;
    char lastTwo[2] = "00";
    do 
    {
        char* curr_byte = malloc(1);
        int charsRead = read(fd, curr_byte, 1);
        if (charsRead){
            dynarray_insert(all_bytes, curr_byte);
            if (charsRead < 0)
            {
                perror("Failed to read");
                exit(1);
            }
            bytes_read++;
        }
        if (bytes_read > 1)
        {
            lastTwo[0] = *(char*)dynarray_get(all_bytes, bytes_read - 2);
            lastTwo[1] = *(char*)dynarray_get(all_bytes, bytes_read - 1);
        }
    }while((lastTwo[0] != '\n') && (lastTwo[1] != '\n'));
    int temp;
    temp = read(fd, &temp, 1);
    char* buffer = malloc(bytes_read);
    int i;
    for (i = 0; i < bytes_read; ++i)
    {
        char* curr_char = (char*)dynarray_get(all_bytes, i);
        buffer[i] = *curr_char;
        free(curr_char);
    }
    buffer[bytes_read - 1] = '\0';
    dynarray_free(all_bytes);
    return buffer;
}

char** AllocateResponse()
{
    return calloc(sizeof(char*), 256);
}

void CleanResponse(char** resp)
{
    int i;
    for (i = 0; i < 256; ++i) {
        if (resp[i])
        {
            free(resp[i]);
            resp[i] = NULL;
        }
    }
}

void FreeResponse(char** resp)
{
    CleanResponse(resp);
    free(resp);
}

int GetResponse(int fd, char** resp, int id)
{
    CleanResponse(resp);
    char* message = GetMessage(fd);
    int terms = 0;
    if (ValidateResponse(message, id))
    {
        char temp[256][256];
        char* save_ptr;
        char* token;
        token = strtok_r(message, " ", &save_ptr);
        while (token != NULL)
        {
            memcpy(temp[terms], token, strlen(token) + 1);
            token = strtok_r(NULL, " ", &save_ptr);
            terms++;
        }
        int i;
        for (i = 0; i < terms; ++i) {
            resp[i] = malloc(strlen(temp[i]) + 1);
            memcpy(resp[i], temp[i], strlen(temp[i]) + 1);
        }
    }
    free(message);
    return terms;
}

void StartEngine(Engine* engine, char* engine_exc)
{
    int to_engine[2];
    int from_engine[2];
    int result = pipe(to_engine);
    while (result == -1)
        result = pipe(to_engine);
    result = pipe(from_engine);
    if (result == -1)
        result = pipe(from_engine);
    pid_t child_pid = fork();
    if (child_pid)
    {
        close(ENGINE_READ);
        close(ENGINE_WRITE);
        engine->read = CONTROLLER_READ;
        engine->write = CONTROLLER_WRITE;
        engine->pid = child_pid;

        SendName(CONTROLLER_WRITE, 1);
        char** response = AllocateResponse();
        int n_terms = GetResponse(CONTROLLER_READ, response, 1);
        int bytes;
        if (n_terms)
        {
            memcpy(engine->name, response[1], strlen(response[1]) + 1);
            bytes = strlen(response[1]);
        }
        int i;
        for (i = 2; i < n_terms; ++i) {
            sprintf(engine->name + bytes, " %s", response[i]);
            bytes += strlen(response[i]);
        }
        CleanResponse(response);

        SendVersion(CONTROLLER_WRITE, 1);
        n_terms = GetResponse(CONTROLLER_READ, response, 1);
        if (n_terms)
            memcpy(engine->version, response[1], strlen(response[1]) + 1);
        CleanResponse(response);
        FreeResponse(response);
    }
    else
    {
        dup2(ENGINE_READ,  0);
        dup2(ENGINE_WRITE, 1);
        close(CONTROLLER_READ);
        close(CONTROLLER_WRITE);
        char** args = calloc(sizeof(char*), 100);

        int i = 0;
        char* save_ptr;
        char* token;
        token = strtok_r(engine_exc, " ", &save_ptr);
        while (token != NULL)
        {
            args[i] = malloc(strlen(token) + 1);
            memcpy(args[i], token, strlen(token) + 1);
            token = strtok_r(NULL, " ", &save_ptr);
            ++i;
        }
        execvp(args[0], args);
    }
}

void SendName(int fd, int id)
{
    char message[256];
    sprintf(message, "%d name\n", id);
    if (write(fd, message, strlen(message)) < 0)
        fprintf(stderr, "error writing command to engine");
}

void SendVersion(int fd, int id)
{
    char message[256];
    sprintf(message, "%d version\n", id);
    if (write(fd, message, strlen(message)) < 0)
        fprintf(stderr, "error writing command to engine");
}

void SendClearBoard(int fd, int id)
{
    char message[256];
    sprintf(message, "%d clear_board\n", id);
    if (write(fd, message, strlen(message)) < 0)
        fprintf(stderr, "error writing command to engine");
}

void SendBoardsize(int fd, int id, int size)
{
    char message[256];
    sprintf(message, "%d boardsize %d\n", id, size);
    if (write(fd, message, strlen(message)) < 0)
        fprintf(stderr, "error writing command to engine");
}

void SendKomi(int fd, int id, float new_komi)
{
    char message[256];
    sprintf(message, "%d komi %f\n", id, new_komi);
    if (write(fd, message, strlen(message)) < 0)
        fprintf(stderr, "error writing command to engine");
}

void SendHandicap(int fd, int id, int new_handicap)
{
    char message[256];
    sprintf(message, "%d fixed_handicap %d\n", id, new_handicap);
    if (write(fd, message, strlen(message)) < 0)
        fprintf(stderr, "error writing command to engine");
}

void SendGenmove(int fd, int id, char color)
{
    char message[256];
    sprintf(message, "%d genmove %c\n", id, color);
    if (write(fd, message, strlen(message)) < 0)
        fprintf(stderr, "error writing command to engine");
}

void SendPlay(int fd, int id, Move move, int size)
{
    char* letters = "ABCDEFGHJKLMNOPQRST";
    char message [256];
    if ((move.p.row == -1) && (move.p.col == -1))
        sprintf(message, "%d play %c pass\n", id, move.color);
    else
        sprintf(message, "%d play %c %c%d\n", id, move.color, letters[move.p.col],
                size - move.p.row);
    if (write(fd, message, strlen(message)) < 0)
        fprintf(stderr, "error writing command to engine");
}


void SendFinalScore(int fd, int id)
{
    char message[256];
    sprintf(message, "%d final_score\n", id);
    if (write(fd, message, strlen(message)) < 0)
        fprintf(stderr, "error writing command to engine");
}
