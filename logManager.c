#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "logManager.h"

//init
void initLogBuffer(LogBuffer *logBuffer){
    pthread_mutex_init(&logBuffer->mutex, NULL);
    pthread_cond_init(&logBuffer->emptyCondition, NULL);
    pthread_cond_init(&logBuffer->fullCondition, NULL);
    logBuffer->first = 0;
    logBuffer->last = 0;
    logBuffer->count = 0;
}

//add log entry to log logic
void addLogEntry(LogBuffer *logBuffer, const char *entry){
    pthread_mutex_lock(&logBuffer->mutex);
    while(logBuffer->count == LOG_BUFFER_SIZE){
        pthread_cond_wait(&logBuffer->fullCondition, &logBuffer->mutex);
    }

    //allocate mem
    logBuffer->buffer[logBuffer->last] = malloc(strlen(entry) + 1);

    //cp log entry to mem and update position
    strcpy(logBuffer->buffer[logBuffer->last], entry);
    logBuffer->last = (logBuffer->last + 1) % LOG_BUFFER_SIZE;
    logBuffer->count++;

    pthread_cond_signal(&logBuffer->emptyCondition);
    pthread_mutex_unlock(&logBuffer->mutex);
}

//logging format function
void createLogEntry(LogBuffer *logBuffer, int clientSocket, const char *word, const char *response, time_t arrivalTime, int priority) {
    char logEntry[1024];
    sprintf(logEntry, "Client: %d Priority: %d Time: %ld Word: %s Response: %s\n",
            clientSocket, priority, arrivalTime, word, response);
    printf("Adding log entry: %s\n",logEntry);
    addLogEntry(logBuffer, logEntry);
}


//thread function
void *logManagerFunction(void *args){
    LogBuffer *logBuffer = (LogBuffer *)args;

    while(1){
        pthread_mutex_lock(&logBuffer->mutex);
        while(logBuffer->count == 0){
            pthread_cond_wait(&logBuffer->emptyCondition, &logBuffer->mutex);
        }

        //same circular queue logic as connection buffer
        char *logEntry = logBuffer->buffer[logBuffer->first];
        logBuffer->first = (logBuffer->first + 1) % LOG_BUFFER_SIZE;
        logBuffer->count--;

        pthread_mutex_unlock(&logBuffer->mutex);

        //write the log entry to the log file
        FILE *logFile = fopen("log.txt", "a");
        if(logFile){
            fprintf(logFile, "%s\n", logEntry);
            fclose(logFile);
        }

        free(logEntry);
    }

    return NULL;
}

