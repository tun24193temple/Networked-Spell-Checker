#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

#include <pthread.h>

#define LOG_BUFFER_SIZE 100

//logBuffer struct
typedef struct{
    pthread_mutex_t mutex;
    pthread_cond_t emptyCondition;
    pthread_cond_t fullCondition;
    char *buffer[LOG_BUFFER_SIZE];
    int first;
    int last;
    int count;
}LogBuffer;

void initLogBuffer(LogBuffer *logBuffer);

void addLogEntry(LogBuffer *logBuffer, const char *entry);

void createLogEntry(LogBuffer *logBuffer, int clientSocket, const char *word, const char *response, time_t arrivalTime, int priority);

void *logManagerFunction(void *args);

#endif 