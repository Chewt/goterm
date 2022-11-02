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
        dynarray_insert(all_bytes, curr_byte);
        if (charsRead < 0)
        {
            perror("Failed to read");
            exit(1);
        }
        bytes_read++;
        if (bytes_read > 1)
        {
            lastTwo[0] = *(char*)dynarray_get(all_bytes, bytes_read - 2);
            lastTwo[0] = *(char*)dynarray_get(all_bytes, bytes_read - 1);
        }
    }while(lastTwo[0] != '\n' && lastTwo[1] != '\n');
    char* buffer = malloc(bytes_read);
    int i;
    for (i = 0; i < bytes_read; ++i)
    {
        char* curr_char = (char*)dynarray_get(all_bytes, i);
        buffer[i] = *curr_char;
        free(curr_char);
    }
    buffer[bytes_read - 2] = '\0';
    dynarray_free(all_bytes);
    return buffer;
}

int GetResponse(int fd, char** resp, int id)
{
    char* message = GetMessage(fd);
    int terms = 0;
    if (ValidateResponse(message, id))
    {
        char temp[256][256];
        char* save_ptr;
        char* token;
        token = strtok_r(message, " ", &save_ptr);
        token = strtok_r(NULL, " ", &save_ptr);
        while (token != NULL)
        {
            terms++;
            memcpy(temp[terms - 1], token, strlen(token) + 1);
            token = strtok_r(NULL, " ", &save_ptr);
        }
        resp = malloc(sizeof(char*) * terms);
        int i;
        for (i = 0; i < terms; ++i) {
            resp[i] = malloc(strlen(temp[i]) + 1);
            memcpy(resp[i], temp[i], strlen(temp[i]) + 1);
        }
    }
    return terms;
}

void start_engine(Engine* engine, char* engine_exc)
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
        char** message;
        int n_terms = GetResponse(CONTROLLER_READ, message, 1);
        if (n_terms)
            memcpy(engine->name, message[0], strlen(message[0]) + 1);
        free(message);

        SendVersion(CONTROLLER_WRITE, 1);
        n_terms = GetResponse(CONTROLLER_READ, message, 1);
        if (n_terms)
            memcpy(engine->version, message[0], strlen(message[0]) + 1);
        free(message);
    }
    else
    {
        dup2(ENGINE_READ,  0);
        dup2(ENGINE_WRITE, 1);

        close(CONTROLLER_READ);
        close(CONTROLLER_WRITE);

        char* args[] = {engine_exc, NULL};
        execvp(engine_exc, args);
    }
}

void SendName(int fd, int id)
{
    char message[256];
    sprintf(message, "%d name\n", id);
    write(fd, message, strlen(message));
}

void SendVersion(int fd, int id)
{
    char message[256];
    sprintf(message, "%d version\n", id);
    write(fd, message, strlen(message));
    Engine e;
}
