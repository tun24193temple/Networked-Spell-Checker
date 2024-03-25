#include <pthread.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include "connectionBuffer.h"

//init
void initConnBuffer(ConnBuffer *buffer, int bufferSize){
    pthread_mutex_init(&buffer->mutex, NULL);
    pthread_cond_init(&buffer->emptyCondition, NULL);
    pthread_cond_init(&buffer->fullCondition, NULL);
    buffer->buffer = malloc(bufferSize * sizeof(ConnData));
    buffer->first = 0;
    buffer->last = 0;
    buffer->count = 0;
    buffer->size = bufferSize;
}

//add connection
void addConnData(ConnBuffer *buffer, ConnData data){
    printf("Adding Client %d...\n", data.socket);
    pthread_mutex_lock(&buffer->mutex);

    //wait if buffer full
    while(buffer->count == buffer->size){
        pthread_cond_wait(&buffer->fullCondition, &buffer->mutex);
    }

    //add conn data to buffer, move last to next position, increment count
    buffer->buffer[buffer->last] = data; 
    buffer->last = (buffer->last + 1) % buffer->size;
    buffer->count++;

    pthread_cond_signal(&buffer->emptyCondition);
    pthread_mutex_unlock(&buffer->mutex);
    
}

//get client
ConnData getConnData(ConnBuffer *buffer){
    printf("Getting Client from buffer...\n");
    pthread_mutex_lock(&buffer->mutex);
    while(buffer->count == 0){
        pthread_cond_wait(&buffer->emptyCondition, &buffer->mutex);
    }

    int hpIndex = -1; //index of connection in buffer with highest priority
    int currenthp = 0; //initialize current highest priority value

    //iterate buffer to find highest priority connection
    for(int i = 0; i < buffer->count; i++){
        int currentIndex = (buffer->first + i) % buffer->size;
        if(buffer->buffer[currentIndex].priority > currenthp){
            hpIndex = currentIndex;
            currenthp = buffer->buffer[currentIndex].priority;
        }
    }

    //swap highest priority connection with first if they are different
    if(hpIndex != buffer->first){
        ConnData temp = buffer->buffer[buffer->first];
        buffer->buffer[buffer->first] = buffer->buffer[hpIndex];
        buffer->buffer[hpIndex] = temp;
    }

    //now the highest priority connection is at the first
    ConnData data = buffer->buffer[buffer->first];
    buffer->first = (buffer->first + 1) % buffer->size;
    buffer->count--;

    pthread_cond_signal(&buffer->fullCondition);
    pthread_mutex_unlock(&buffer->mutex);

    return data;
}

ConnData createDummyConnData(int socket, int priority) {
    ConnData data;
    data.socket = socket;
    data.priority = priority;
    return data;
}

