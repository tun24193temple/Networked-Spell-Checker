#ifndef CONNECTION_BUFFER_H
#define CONNECTION_BUFFER_H

#include <pthread.h>
#include <time.h>
#include <stdbool.h>

//data for client's connection
typedef struct{
    int socket;
    time_t arrivalTime;
    int priority;
}ConnData;

//connection buffer
typedef struct{
    pthread_mutex_t mutex;
    pthread_cond_t emptyCondition;
    pthread_cond_t fullCondition;
    ConnData *buffer;
    int first;
    int last;
    int count;
    int size;  
}ConnBuffer;

void initConnBuffer(ConnBuffer *buffer, int bufferSize);

void addConnData(ConnBuffer *buffer, ConnData data);

ConnData getConnData(ConnBuffer *buffer);

#endif 
